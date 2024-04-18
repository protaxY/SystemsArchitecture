export DB_HOST=postgres
export DB_PORT=5432
export DB_LOGIN=stud
export DB_PASSWORD=stud
export DB_DATABASE=archdb

python3 lab2/gen.py $1
./lab2/build/app