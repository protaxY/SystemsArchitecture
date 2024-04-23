import sys
import random

import time
import datetime

from faker import Faker
import bson
from pymongo import MongoClient

if len(sys.argv) == 3:
    posts_number = int(sys.argv[1])
    users_number = int(sys.argv[2])
else:
    posts_number = 10
    users_number = 3

fake = Faker()

try:
    client = MongoClient("mongodb", 27017)
    db = client.arch
    posts_collection = db.posts
    posts_collection.drop()
except:
    raise Exception('Can`t establish connection to database')

for i in range(posts_number):
    post = {
        "id": bson.Int64(i),
        "author_id": bson.Int64(random.randint(1, users_number + 1)),
        "text_content": fake.text(max_nb_chars=100),
        "send_time": fake.past_datetime().strftime('%Y %b %dd %H:%M:%S') + ' GMT',
    }

    posts_collection.insert_one(post)