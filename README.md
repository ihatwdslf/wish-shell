# wish – простий Unix-термінал

Лабораторна робота з дисципліни «Операційні системи».
Реалізація завдання [processes-shell](https://github.com/remzi-arpacidusseau/ostep-projects/tree/master/processes-shell) з курсу OSTEP мовою C.

## Можливості

- інтерактивний режим (`./wish`) і пакетний режим (`./wish batch.txt`)
- запуск програм через `fork`, `execv` і `waitpid`
- пошук програм у папках зі списку path (за замовчуванням `/bin`)
- вбудовані команди `exit`, `cd`, `path`
- перенаправлення stdout і stderr у файл: `ls -la > out.txt`
- паралельні команди: `cmd1 & cmd2 & cmd3`

## Збірка

    make

## Запуск

    ./wish            # інтерактивний режим
    ./wish batch.txt  # пакетний режим

## Тести

    ./test-wish.sh -c

Тести взято з репозиторію [ostep-projects](https://github.com/remzi-arpacidusseau/ostep-projects).
Тест №3 під час запуску перезаписує файл `tests/3.err`; повернути оригінал можна командою `git checkout tests/3.err`.