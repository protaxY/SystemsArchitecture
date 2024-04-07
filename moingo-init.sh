set -e

mongo <<EOF
db = db.getSiblingDB('arch')
db.createCollection('messages')
db.messages.createIndex({"id": -1}) 
db.messages.createIndex({"send_date": -1}) 
EOF