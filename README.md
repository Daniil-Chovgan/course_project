# typst-nelder-mead-wasm

Высокопроизводительный плагин для [Typst](https://typst.app/), реализующий алгоритм Нелдера-Мида для многомерной оптимизации. Вычислительное ядро написано на C, скомпилировано в WebAssembly и интегрировано в Typst через автоматизированный бинарный протокол обмена данными.

## Особенности
- **Скорость**: Компиляция через Emscripten с флагами `-O3`, отключение ФС и runtime-ассертов.
- **Безопасность**: Выполнение в изолированной WASI-песочнице без доступа к файловой системе и сетевым вызовам.
- **Автогенерация протокола**: Сериализация/десериализация структур генерируется автоматически из `.prot` файлов с помощью [WebAssembly-protocol-generator](https://github.com/Robotechnic/WebAssembly-protocol-generator).
- **Демо-модуль**: Оптимизация функции Розенброка с точностью до `1e-8`.
- **Удобный API**: Нативный вызов из Typst без ручной работы с байтовыми буферами.

## Требования
- [Typst](https://typst.app/) `≥ 0.11.0`
- [Emscripten SDK](https://emscripten.org/) (`emcc` в `PATH`)
- `make`
- [WebAssembly-protocol-generator](https://github.com/Robotechnic/WebAssembly-protocol-generator) (бинарник `wasmpg`)
- [wasi-stub](https://github.com/bytecodealliance/wasm-tools) (рекомендуется для заглушки системных вызовов)

## Сборка
+ Укажите пути к утилитам в `Makefile`:
	```makefile
	WASMPG = /path/to/wasmpg
	WASI_STUB = /path/to/wasi-stub
	```
+ Запустите сборку:
  ```
  make clean && make
  ```
В директории появятся:
`nelder_mead.wasm` (готовый плагин)
protocol/ (сгенерированные `protocol.c`, `protocol.h`, `protocol.typ`)

## Использование
Подключите плагин и вызовите функцию оптимизации:

```typst
#import "plugin.typ": nelder-mead

#let result = nelder-mead(
  initial_guess: (-1.2, 1.0),
  tolerance: 1e-7,
  max_iterations: 5000
)

Найденный минимум: #result.minimum_point \
Значение функции: #result.minimum_value \
Итераций: #result.iterations
```

## Формат протокола (.prot)
Протокол описывается в файле `nelder_mead.prot`:
```
struct Params {
    float initial_guess[];
    float tolerance;
    int max_iterations;
}

struct Result {
    float minimum_point[];
    float minimum_value;
    int iterations;
    bool success;
}

protocol C Request { Params params; }
protocol Typst Response { Result result; }
```

Генератор автоматически создает:
В C: struct Params, struct Result, encode_Request(), decode_Response()
В Typst: encode-Request(), decode-Response() (имена функций соответствуют регистру протокола)
Синтаксис массивов: тип имя[];. Опциональные поля: тип? имя;.

## Структура проекта
```
.
├── main.c                # Ядро на C (алгоритм Нелдера-Мида, целевая функция)
├── nelder_mead.prot      # Определение протокола обмена данными
├── Makefile              # Сборка, генерация кода, WASI-заглушки
├── plugin.typ            # Типст-обертка (сериализация, вызов WASM, десериализация)
├── usage.typ             # Минимальный пример вызова
├── demo.typ              # Полноценный демонстрационный документ
└── README.md             # Документация проекта
```


---

## `usage.typ` (Базовый пример)

```typst
#set page(margin: 2cm)
#set text(font: "Linux Libertine")

= Базовый вызов плагина оптимизации
#import "plugin.typ": nelder-mead

#let start = (-1.2, 1.0)
#let res = nelder-mead(
  initial_guess: start,
  tolerance: 1e-7,
  max_iterations: 5000
)

== Результат вычислений
#table(
  columns: 2,
  align: (left, center),
  [*Параметр*], [*Значение*],
  [$x_1$], #res.minimum_point.at(0).round(6),
  [$x_2$], #res.minimum_point.at(1).round(6),
  [$f(x)$], #res.minimum_value.round(8),
  [Итераций], #res.iterations
)

_Глобальный минимум функции Розенброка: $(1, 1)$, $f = 0$_
