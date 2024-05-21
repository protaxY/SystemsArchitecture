import sys
import os

from faker import Faker
import psycopg2

if len(sys.argv) == 2:
    rows_number = int(sys.argv[1])
else:
    rows_number = 10

# для упрощения удаления и обновления информации о пользователе,
# генерируемые пользователи все имею одинаковый пароль: password
default_hased_password = 'bbf7c886e7fa45ac432a58a062c50acfbd03c680a2374bc503fe7b96c7a7ee3e18bb2fd9fdd0ad69071c6018a76893bc469bd385c9307a7b83c42c2f25413134bccbe99f76be85360ac5ccba92977e9d05107182dd9d527abbb7012d652393bbbbf24daa0c9bd0629ca9f43bd264a721cab08115376e6ae2131c9cc619315f08'

fake = Faker()
try:
    conn = psycopg2.connect(dbname=os.environ['DB_DATABASE'], user=os.environ['DB_LOGIN'], password=os.environ['DB_PASSWORD'], host=os.environ['DB_HOST'])
except:
    raise Exception('Can`t establish connection to database')

user_logins = []

with conn.cursor() as curs:
    try:
        curs.execute('DROP TABLE IF EXISTS users')
        
        curs.execute('CREATE TABLE IF NOT EXISTS users (id SERIAL, '
                    'first_name VARCHAR(256) NOT NULL, '
                    'last_name VARCHAR(256) NOT NULL, '
                    'login VARCHAR(256) NOT NULL, '
                    'password VARCHAR(256) NOT NULL, '
                    'email VARCHAR(256) NULL, '
                    'title VARCHAR(1024) NULL)')

        insert_command = 'INSERT INTO users (first_name, last_name, email, title, login, password) VALUES '

        for _ in range(rows_number):
            user_login = fake.unique.user_name()
            values = f" ('{fake.first_name()}', '{fake.last_name()}', '{fake.ascii_free_email()}', '{fake.text(max_nb_chars=100)}', '{user_login}', '{default_hased_password}'),"
            insert_command += values

            user_logins.append(user_login)

        insert_command = insert_command[:-1]

        curs.execute(insert_command)
        conn.commit()
    except Exception as e:
        print(e)

conn.close()

with open('user_logins.txt', 'w') as f:
    for login in user_logins:
        f.write(login + '\n')