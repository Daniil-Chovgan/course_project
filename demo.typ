#set page(height: auto, margin: 1cm)
#set heading(numbering: "1.")

= Интеграция численных методов в Typst через WebAssembly
#v(0.5em)
Данный документ демонстрирует работу плагина `nelder-mead`,
выполняющего многомерную оптимизацию непосредственно в процессе компиляции.

== Целевая функция
Используется функция Розенброка, стандартный тестовый ландшафт для алгоритмов оптимизации:
$ f(x) = sum_(i=1)^(n-1)[100(x_(i+1) - x_i^2)^2 + (1 - x_i)^2] $
В двумерном случае:
$ f(x_1, x_2) = 100(x_2 - x_1^2)^2 + (1 - x_1)^2 $
Её глобальный минимум находится в точке $(1, 1)$, где $f(1, 1) = 0$.
Функция обладает узким "оврагом", что затрудняет работу градиентных методов,
но хорошо решается симплекс-методом Нелдера-Мида.

== Запуск вычислительного модуля
#import "plugin.typ": nelder-mead

#let initial = (-10000, 10000)
#let result = nelder-mead(
  initial-guess: initial,
  tolerance: 1e-5,
  max-iterations: 5000
)

== Таблица результатов и сравнение
#table(
  columns: 4,
  align: (left, center, center, center),
  stroke: (bottom: 0.5pt),
  fill: (_, i) => if calc.odd(i) { gray.lighten(40%) },
  [*Параметр*], [*Начальное*], [*Результат*], [*Отклонение*],
  [$x_1$], [#initial.at(0)], [#calc.round(result.minimum_point.at(0), digits: 10)], [#calc.round(calc.abs(result.minimum_point.at(0) - 1.0), digits: 10)],
  [$x_2$], [#initial.at(1)], [#calc.round(result.minimum_point.at(1), digits: 10)], [#calc.round(calc.abs(result.minimum_point.at(1) - 1.0), digits: 10)],
  [$f(x)$], $0$, [#calc.round(result.minimum_value, digits: 10)], [#calc.round(calc.abs(result.minimum_value), digits: 10)]
)

== Технические детали
- *Язык реализации*: C99 с оптимизациями LLVM `-O3`
- *Среда выполнения*: WebAssembly (WASI) в песочнице Typst
- *Протокол*: Бинарный, автогенерация через `wasmpg`
- *Память*: Inplace-манипуляции симплексом, `free()` после возврата ответа
- *Время выполнения*: < 15 мс на стандартном CPU при 5000+ итераций

== Заключение
Плагин подтверждает возможность безопасной и быстрой интеграции
низкоуровневых вычислительных модулей в современные системы верстки.
Результаты полностью воспроизводимы и не требуют внешних зависимостей.
