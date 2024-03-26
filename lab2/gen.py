import sys
import os

from faker import Faker
import psycopg2

rows_number = int(sys.argv[1])
# для упрощения удаления и обновления информации о пользователе,
# генерируемые пользователи все имею одинаковый пароль: password
default_hased_password = 'bbf7c886e7fa45ac432a58a062c50acfbd03c680a2374bc503fe7b96c7a7ee3e18bb2fd9fdd0ad69071c6018a76893bc469bd385c9307a7b83c42c2f25413134bccbe99f76be85360ac5ccba92977e9d05107182dd9d527abbb7012d652393bbbbf24daa0c9bd0629ca9f43bd264a721cab08115376e6ae2131c9cc619315f08'

fake = Faker()
try:
    conn = psycopg2.connect(dbname=os.environ['DB_DATABASE'], user=os.environ['DB_LOGIN'], password=os.environ['DB_PASSWORD'], host=os.environ['DB_HOST'])
except:
    print('Can`t establish connection to database')

with conn.cursor() as curs:
    try:
        curs.execute('CREATE TABLE IF NOT EXISTS users (id SERIAL, '
                    'first_name VARCHAR(256) NOT NULL, '
                    'last_name VARCHAR(256) NOT NULL, '
                    'login VARCHAR(256) NOT NULL, '
                    'password VARCHAR(256) NOT NULL, '
                    'email VARCHAR(256) NULL, '
                    'title VARCHAR(1024) NULL)')

        insert_command = 'INSERT INTO users (first_name,last_name,emxail,title,login,password) VALUES '

        for _ in range(rows_number):
            values = f" ('{fake.first_name()}', '{fake.last_name()}', '{fake.ascii_free_email()}', '{fake.text(max_nb_chars=100)}', '{fake.unique.user_name()}', '{default_hased_password}'),"
            insert_command += values
        insert_command = insert_command[:-1]

        curs.execute(insert_command)
        conn.commit()
    except:
        print('somethong wrong')

conn.close() # закрываем соединение