#include "protocol.h"
#include <emscripten.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

// Целевая функция (Функция Розенброка).
double objective_function(const float *x, int n) {
  double sum = 0.0;
  for (int i = 0; i < n - 1; i++) {
    double xi = x[i];
    double xi1 = x[i + 1];
    sum += 100.0 * pow(xi1 - xi * xi, 2.0) + pow(1.0 - xi, 2.0);
  }
  return sum;
}

typedef struct {
  float *vertex;
  float value;
} SimplexPoint;

int compare_points(const void *a, const void *b) {
  float fa = ((const SimplexPoint *)a)->value;
  float fb = ((const SimplexPoint *)b)->value;
  return (fa > fb) - (fa < fb);
}

void run_nelder_mead(Request *req, Response *res) {
  int n = req->params.initial_guess_len;
  if (n <= 0) {
    res->result.success = 0;
    res->result.minimum_point = NULL;
    res->result.minimum_point_len = 0;
    res->result.minimum_value = 0.0;
    res->result.iterations = 0;
    return;
  }

  float *initial_guess = req->params.initial_guess;
  float tol = req->params.tolerance;
  int max_iter = req->params.max_iterations;

  // Инициализация симплекса (n+1 точка в n-мерном пространстве)
  SimplexPoint *simplex =
      (SimplexPoint *)malloc((n + 1) * sizeof(SimplexPoint));
  for (int i = 0; i < n + 1; i++) {
    simplex[i].vertex = (float *)malloc(n * sizeof(float));
    for (int j = 0; j < n; j++) {
      simplex[i].vertex[j] = initial_guess[j];
    }
  }

  // Возмущение для формирования начального симплекса
  for (int i = 0; i < n; i++) {
    if (initial_guess[i] == 0.0f) {
      simplex[i + 1].vertex[i] = 0.00025f;
    } else {
      simplex[i + 1].vertex[i] = initial_guess[i] * 1.05f;
    }
  }

  for (int i = 0; i < n + 1; i++) {
    simplex[i].value = objective_function(simplex[i].vertex, n);
  }

  float alpha = 1.0f, gamma = 2.0f, rho = 0.5f, sigma = 0.5f;
  float *centroid = (float *)malloc(n * sizeof(float));
  float *xr = (float *)malloc(n * sizeof(float));
  float *xe = (float *)malloc(n * sizeof(float));
  float *xc = (float *)malloc(n * sizeof(float));

  int iter = 0;
  int success = 0;

  while (iter < max_iter) {
    qsort(simplex, n + 1, sizeof(SimplexPoint), compare_points);

    // Критерий остановки: разница между лучшей и худшей точкой
    float max_diff = 0.0f;
    for (int i = 1; i < n + 1; i++) {
      float diff = fabsf(simplex[i].value - simplex[0].value);
      if (diff > max_diff)
        max_diff = diff;
    }
    if (max_diff < tol) {
      success = 1;
      break;
    }

    // Вычисление центроида (без худшей точки simplex[n])
    for (int j = 0; j < n; j++) {
      centroid[j] = 0.0f;
      for (int i = 0; i < n; i++) {
        centroid[j] += simplex[i].vertex[j];
      }
      centroid[j] /= n;
    }

    // Отражение
    for (int j = 0; j < n; j++) {
      xr[j] = centroid[j] + alpha * (centroid[j] - simplex[n].vertex[j]);
    }
    float fr = objective_function(xr, n);

    if (fr < simplex[0].value) {
      // Растяжение
      for (int j = 0; j < n; j++) {
        xe[j] = centroid[j] + gamma * (xr[j] - centroid[j]);
      }
      float fe = objective_function(xe, n);
      if (fe < fr) {
        memcpy(simplex[n].vertex, xe, n * sizeof(float));
        simplex[n].value = fe;
      } else {
        memcpy(simplex[n].vertex, xr, n * sizeof(float));
        simplex[n].value = fr;
      }
    } else if (fr < simplex[n - 1].value) {
      memcpy(simplex[n].vertex, xr, n * sizeof(float));
      simplex[n].value = fr;
    } else {
      // Сжатие
      int shrink = 0;
      if (fr < simplex[n].value) {
        for (int j = 0; j < n; j++) {
          xc[j] = centroid[j] + rho * (xr[j] - centroid[j]);
        }
        float fc = objective_function(xc, n);
        if (fc <= fr) {
          memcpy(simplex[n].vertex, xc, n * sizeof(float));
          simplex[n].value = fc;
        } else {
          shrink = 1;
        }
      } else {
        for (int j = 0; j < n; j++) {
          xc[j] = centroid[j] + rho * (simplex[n].vertex[j] - centroid[j]);
        }
        float fc = objective_function(xc, n);
        if (fc < simplex[n].value) {
          memcpy(simplex[n].vertex, xc, n * sizeof(float));
          simplex[n].value = fc;
        } else {
          shrink = 1;
        }
      }

      // Схлопывание
      if (shrink) {
        for (int i = 1; i < n + 1; i++) {
          for (int j = 0; j < n; j++) {
            simplex[i].vertex[j] =
                simplex[0].vertex[j] +
                sigma * (simplex[i].vertex[j] - simplex[0].vertex[j]);
          }
          simplex[i].value = objective_function(simplex[i].vertex, n);
        }
      }
    }
    iter++;
  }

  qsort(simplex, n + 1, sizeof(SimplexPoint), compare_points);

  // Формирование ответа
  // Память для minimum_point будет освобождена автоматически функцией
  // free_Response
  res->result.minimum_point = simplex[0].vertex;
  res->result.minimum_point_len = n;
  res->result.minimum_value = simplex[0].value;
  res->result.iterations = iter;
  res->result.success = success;

  // Очистка вспомогательной памяти
  for (int i = 1; i < n + 1; i++)
    free(simplex[i].vertex);
  free(simplex);
  free(centroid);
  free(xr);
  free(xe);
  free(xc);
}

// Экспортируемая функция Wasm
EMSCRIPTEN_KEEPALIVE
int optimize(size_t buffer_len) {
  Request req = {0};
  if (decode_Request(buffer_len, &req) != 0)
    return 1;

  Response res = {0};
  run_nelder_mead(&req, &res);

  int err = encode_Response(&res);

  // Освобождение памяти, выделенной при декодировании и кодировании
  free_Request(&req);
  free_Response(&res);

  return err;
}
