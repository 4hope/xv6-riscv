# Тесты для программы

## 1. Проверка работы в forground режиме
```bash
# В первой консоли
./program

# В другой консоли
echo "Message 1" > /tmp/exo-server
echo "Message 2" > /tmp/exo-server

# Потом делаем Cntrl + C в первой консоли (вызываем SIGINT)
```

Ожидаемый вывод:
```bash
FIFO exists, will use it
SIGALRM: Waiting for data...
SIGALRM: Waiting for data...
SIGALRM: Waiting for data...
SIGALRM: Waiting for data...
SIGALRM: Waiting for data...
SIGALRM: Waiting for data...
SIGALRM: Waiting for data...
^CSIGINT: user request, read the data and finish the work
Statistics: 2 messages, 0 bytes, 7 alarms
```

## 2. Проверка работы в демон-режиме
```bash
# В первой консоли
./program -d

# Во второй консоли
echo "Message 1" > /tmp/exo-server
echo "Message 2" > /tmp/exo-server

# Проверка содержимого лог-файла во второй консоли
cat /tmp/exo-server.txt
```

Ожидаемый вывод в файл:
```bash
SIGALRM: Waiting for data...
Message 1
SIGALRM: Waiting for data...
Message 2
SIGALRM: Waiting for data...
SIGALRM: Waiting for data...
SIGALRM: Waiting for data...
SIGALRM: Waiting for data...
SIGALRM: Waiting for data...
SIGALRM: Waiting for data...
SIGALRM: Waiting for data...
SIGALRM: Waiting for data...
SIGALRM: Waiting for data...
SIGALRM: Waiting for data...
```

## 3. Проверка сигналов
```bash
# В первой консоли
./program

# Во второй консоли
echo "Message 1" > /tmp/exo-server
ps aux | grep ./pp # чтоб узнать pid
kill -QUIT <pid>
kill -USR1 <pid>
kill -HUP <pid>
ps aux | grep ./pp # чтоб узнать новый pid
kill -USR1 <pid>
kill -INT <pid>
cat /tmp/exo-server.txt
```

Ожидаемый вывод в консоль:
```bash
FIFO created
SIGALRM: Waiting for data...
Message 1
SIGALRM: Waiting for data...
SIGALRM: Waiting for data...
SIGALRM: Waiting for data...
SIGALRM: Waiting for data...
SIGALRM: Waiting for data...
Statistics: 1 messages, 10 bytes, 6 alarms
SIGALRM: Waiting for data...
```

Ожидаемый вывод в файл:
```bash
SIGHUP: switching to daemon mode
Statistics: 0 messages, 0 bytes, 2 alarms
SIGALRM: Waiting for data...
SIGALRM: Waiting for data...
Statistics: 0 messages, 0 bytes, 4 alarms
SIGALRM: Waiting for data...
SIGALRM: Waiting for data...
SIGALRM: Waiting for data...
SIGALRM: Waiting for data...
SIGINT: user request, read the data and finish the work
Statistics: 0 messages, 0 bytes, 8 alarms
```

## 4. Проверка сигнала SIGTERM
```bash
# В первой консоли
./program

# Во второй консоли
kill -TERM <pid>
```

Ожидаемый вывод:
```bash
FIFO exists, will use it
SIGALRM: Waiting for data...
SIGALRM: Waiting for data...
SIGTERM: interruption of work at the user's request
Statistics: 0 messages, 0 bytes, 2 alarms
```

## 5. Создание FIFO
```bash
rm /tmp/exo-server
./program
```

Ожидаемый вывод:
```bash
FIFO created
SIGALRM: Waiting for data...
```
