# Краткое описание проекта

Данный проект представляет из себя Сборщик мусора с применением стратегии Mark and Sweep для языка C.

# Использование

## Инициализация сборщика

Сначала необходимо подключить gc.h. Далее необходимо создать сам сборщик мусора. Делается это в с помощью строки 
```cpp
gc_create();
```
Сборщик сам по себе глобальный.

## Работа с памятью

Сама суть данной библиотеки в том, чтобы переопределить стандартные функции malloc, calloc, realloc и free функциями сборщика мусора. Чтобы ими воспользоваться необходимо указать префикс gc_ перед функцией и первым параметром передать адрес сборщика мусора.

## Работа с многопоточностью

Если вы хотите, чтобы какой-либо из потоков учитывался в сборке мусора, то можно это сделать с помощью 
```cpp
gc_register_thread();
```
для своего потока.

Если хотите отвязать поток от проверки, то надо прописать 
```cpp
gc_unregister_thread();
```
для своего потока.

Обратите внимание, что поток, из которого вызвался gc_create(); привязывает себя по умолчанию.

## Очистка мусора

Мусор автоматически собирается каждые gc->allocation_threshold аллокаций. Также предусмотрена возможность ручного вызова сборщика collect_garbage(). Можно также удалить весь сборщик со всеми аллокациями с помощью gc_destruct().

Пример использования сборщика можно найти в файле demo.c.

## Модификация скрипта линкера

Если по какой-то причине линкер будет ругаться на символы _end и _edata, то это значит, что линкер нам их не предоставил. Их надо будет указать в скрипте линкера руками.

# Описание принципа работы

## Мотивация использовния Mark and Sweep

Существуют 4 основные стратегии сборки мусора: reference counting, mark and sweep, mark and copy, mark and compact. Reference counting отваливается, так как он требует переопределения оперетора присваивания, что в языке C невозможно. В C++ это реализовано через std::shared_ptr, так как там есть для этого возможность. Mark and copy и mark and compact требуют перемещения объектов, из-за чего меняются их адреса, а так как в C всё жёстко привязано к адресам, мы позволить себе такое не можем. Остаётся один вариант.

## Описание алгоритма

Когда мы аллоцируем память мы заводим в некотром смысле вершину графа. На этапе mark мы смотрим, какие из ячеек памяти нам доступны. Обход проходит через стек, секцию .data и секцию .bss. Если внутри целиком лежит адрес начала аллоцированной памяти, то данная вершина помечается корневой.

Так как в многопоточной программе стеков может быть больше чем один, то вызвавший поток отправляет сигнал SIGUSR1 всем остальным зарегистрированным потокам, которые ловят этот сигнал и сами делают mark для своего стека.

Далее мы проходим по самому выделенному куску и смотрим, в какие адреса мы можем попасть оттуда. Далее осуществляется поиск в глубину из корневых вершин. Этап mark на этом окончен.

На этапе sweep мы проходим по всем аллоцированным ячейкам и смотрим, достижимы они или нет. Если нет, то освобождаем память.

## Минусы

- Память получается фрагментированной
- Этап mark крайне затратен по времени, однако для слишком больших объектов можно использовать стандартную аллокацию, либо использовать предусмотренную возможность отключения данного куска памяти от проверок алгоритма и ручное управление этим куском.

## Как сломать сборщик
- Хранить адрес не целиком, а, например, побайтово, тогда сборщик посчитает, что данный адрес уже неактуален
- Хранить указатель не на начало выделенной памяти, а со смещением

# Проделанные этапы
- Изучены теоретические материалы
- Подготовлена инфраструктура для написания тестов через gtest
- Написан вспомогательный HashMap для хранения адресов и других данных аллоцированных ячеек
- Написано базовое приближение сборщика мусора для однопоточных программ с ручной сборкой.
- Реализована автоматическая сборка мусора
- Реализована сборка мусора для нескольких потоков
- Добавлена поддержка разлинчых платформ

## TODO:
- Написать тесты