import sys
import random

import time
import datetime

from faker import Faker
import bson
from pymongo import MongoClient

if len(sys.argv) == 3:
    messages_number = int(sys.argv[1])
    users_number = int(sys.argv[2])
else:
    messages_number = 10
    users_number = 3

fake = Faker()

try:
    client = MongoClient("mongodb", 27017)
    db = client.arch
    messages_collection = db.messages
    messages_collection.drop()
except:
    raise Exception('Can`t establish connection to database')

for i in range(messages_number):
    message = {
        "id": bson.Int64(i),
        "sender_id": bson.Int64(random.randint(1, users_number + 1)),
        "receiver_id": bson.Int64(random.randint(1, users_number + 1)),
        "text_content": fake.text(max_nb_chars=50),
        "send_time": fake.past_datetime().strftime('%Y %b %dd %H:%M:%S') + ' GMT',
    }

    messages_collection.insert_one(message)

client.close()