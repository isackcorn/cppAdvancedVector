# Advanced Vector
В данном проекте разработан собственный динамический контейнер, аналогичный `std::vector` по производительности. Реализованы ключевые методы, такие как Resize, PushBack, PopBack, EmplaceBack, Insert, Emplace и Erase, позволяющие удобно манипулировать содержимым контейнера.

## Основные возможности

- Метод `Resize` предоставляет возможность изменить количество элементов в векторе.

- Функция `Reserve` предназначена для предварительного резервирования памяти под элементы вектора, когда примерное количество элементов известно заранее.

- Метод `PushBack` добавляет новое значение в конец вектора, автоматически увеличивая вместимость контейнера вдвое в случае нехватки памяти.

- Метод `PopBack` удаляет последний элемент вектора.

- Метод `EmplaceBack` предоставляет возможность создать элемент вектора в его конце, возвращает ссылку на добавленный элемент.

- Метод `Insert` позволяет вставить элемент в указанную позицию вектора.

- Метод `Emplace` позволяет конструировать элемент вектора в заданной позиции, используется perfect forwarding.

- Метод `Erase` позволяет удалить элемент, на который указывает переданный итератор.

## Сборка
Сборка и установка данного контейнера могут быть осуществлены с использованием любой интегрированной среды разработки (IDE) / командной строки. Необходим компилятор С++ с поддержкой стандарта С++17 и более поздних версий.
