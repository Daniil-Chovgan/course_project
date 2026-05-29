#import "plugin.typ": nelder-mead

// Глобальный минимум функции Розенброка находится в точке (1.0, 1.0)
#let result = nelder-mead(
  initial-guess: (-1000000, 1000000),
  tolerance: 1e-9,
  max-iterations: 5000
)

Найденный минимум: #result.minimum_point \
Значение функции: #result.minimum_value \
Потребовалось итераций: #result.iterations
