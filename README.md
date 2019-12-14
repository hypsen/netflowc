# **netflowc**

Netflow collector V9.

## **Install**
```
mkdir build
cd build
cmake ..
cmake --build .
cpack
apt install netflowc_*version*.deb
```

## **Usage and examples**

### JSON output
```netflowc --port=2055 --stdout```

### JSON output with JQ
```netflowc -r 2055 -o | jq -c '. | select (.IPV4_DST_ADDR == "8.8.8.8") | select(.L4_DST_PORT == 53)'```

### JSON output, write to directory
```netflowc --port=2055 --file --dir=/storage --period=10```

### Write netflow to clickhouse server
```netflowc --port=2055 --clickhouse --dbhost=127.0.0.1 --dbport=9000 --database=netflow --user=collector --password=secret```

### Combine output methods
```netflowc --port=2055 --stdout --file --dir=/storage --period=30 --clickhouse --dbhost=127.0.0.1 --dbport=9000 --database=netflow --user=collector --password=secret```

