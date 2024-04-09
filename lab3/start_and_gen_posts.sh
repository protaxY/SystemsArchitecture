- MONGO_HOST=mongodb
- MONGO_PORT=27017
- MONGO_DATABASE=arch

python3 lab3/gen_posts.py $1 $2
./lab3/post_app/build/app