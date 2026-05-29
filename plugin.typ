#import "protocol.typ": encode-Request, decode-Response

#let wasm-plugin = plugin("nelder_mead.wasm")

#let nelder-mead(
  initial-guess: none,
  tolerance: 1e-5,
  max-iterations: 1000,
) = {
  if initial-guess == none {
    panic("Параметр initial-guess обязателен")
  }
  // Формируем словарь для кодирования.
  let req = (
    params: (
      initial_guess: initial-guess.map(float),
      tolerance: float(tolerance),
      max_iterations: int(max-iterations),
    )
  )

  let bytes = encode-Request(req)
  let res-bytes = wasm-plugin.optimize(bytes)

  if res-bytes.len() == 0 {
    panic("Wasm плагин завершился с ошибкой")
  }

  let (res, _) = decode-Response(res-bytes)
  let result-data = res.result

  if not result-data.success {
    panic("Алгоритм не сошелся за заданное число итераций")
  }

  return (
    minimum_point: result-data.minimum_point,
    minimum_value: result-data.minimum_value,
    iterations: result-data.iterations,
  )
}
