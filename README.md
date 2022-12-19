# producer-consumer problem

a multi-threaded approach to the producer-consumer problem, first introduced by Dijkstra in 1965.

the program may accept input as stdin or through stdin/stdout redirection

### steps to compile
```
gcc --std=c99 -pthread -g producer_consumer.c -o producer_consumer
```

### Examples to run:

for stdin through command line
```
./producer_consumer
```

for redirected stdin from a file
```
./producer_consumer > stdout.txt
```

for redirected stdin from a file, and redirected stdout to a different file
```
./producer_consumer < testfile1.txt > stdout.txt
```


