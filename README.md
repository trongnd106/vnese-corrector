# 1. Run Server Ulfius

## Run command to compile source

```
gcc server.c -o server -lulfius -ljansson -lmysqlclient
```

## Run command to start Server

```
./server
```

## Server Ulfius running on http://127.0.0.1:8080

# 2. Run Server Flask

## Run command to start Server

```
python3 server.py
```

## Server Flask running on http://127.0.0.1:8000

# 3. Run command to create local Database connection

```
docker run --name mysql-corrector \
  -e MYSQL_ROOT_PASSWORD=root \
  -e MYSQL_DATABASE=corrector \
  -p 3306:3306 \
  -d mysql:8.0
```

### When database container do not active

```
sudo systemctl stop mysql
docker start mysql-corrector
```