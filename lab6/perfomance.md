# Лабораторная работа №5
# Data Cache

| Студент | *Федоров Антон Сергеевич* |
|------|------|
| Группа  | *М8О-114М-23* |

## Тест производительности с помощью утилиты wrk

# Тест производительности без использования кэширования
```sh
prota@DESKTOP-D9C0CN3:~/SystemsArchitecture/lab6/wrk_test$ ./test.sh
Running 5m test @ http://localhost:8888
  1 threads and 10 connections
  Thread Stats   Avg      Stdev     Max   +/- Stdev
    Latency   118.02ms  210.39ms   1.54s    90.85%
    Req/Sec    44.33     42.71   141.00     62.96%
  Latency Distribution
     50%   57.15ms
     75%   64.49ms
     90%  223.06ms
     99%  867.99ms
  190 requests in 5.00m, 231.41KB read
  Socket errors: connect 0, read 190, write 0, timeout 48
  Non-2xx or 3xx responses: 145
Requests/sec:      0.63
Transfer/sec:     789.82B
```

# Тест производительности с использованием кэширования (время устаревания: 60 секунд)
```sh
prota@DESKTOP-D9C0CN3:~/SystemsArchitecture/lab6/wrk_test$ ./test.sh
Running 5m test @ http://localhost:8888
  1 threads and 10 connections
  Thread Stats   Avg      Stdev     Max   +/- Stdev
    Latency    29.01ms   92.66ms   1.99s    98.54%
    Req/Sec   126.18     77.30   520.00     71.11%
  Latency Distribution
     50%    1.60ms
     75%   53.97ms
     90%   61.37ms
     99%  315.76ms
  32020 requests in 5.00m, 37.99MB read
  Socket errors: connect 0, read 32020, write 0, timeout 113
  Non-2xx or 3xx responses: 9376
Requests/sec:    106.70
Transfer/sec:    129.62KB
```

# Выводы
Большая разница в два порядка может быть связанна с большим количеством запросов на получение полного списка пользователей (сотня записей), сообщений (тысяча записей) и записей на стене (тысяча записей). В любом случае, кэширование запросов увеличила пропускную способность API-шлюза.