workspace {
    name "Социальная сеть"
    description "Учебное приложение для курса \"Aрхитектуры информационных систем\""

    !identifiers hierarchical

    model {
        properties { 
            structurizr.groupSeparator "/"
        }
        
        user = person "Пользователь социальной сети"
        social_network = softwareSystem "Социальная сеть"{
            description "Сервер социальной сети"

            rest_api_service = container "REST API" {
                description "API взаимодествия пользователя с социальной сетью"
            }

            group "Слой данных" {
                user_database = container "User Database" {
                    description "База данных с информацией о пользователях"
                    technology "PostgreSQL"
                    tags "database"
                }

                user_cache = container "User Cache" {
                    description "Кеш пользовательских данных для ускорения запросов по ключевым полям"
                    technology "Redis"
                    tags "database"
                }

                document_database = container "Document Database" {
                    description "База данных для хранения информации о записях и сообщениях пользователей"
                    technology "MongoDB"
                    tags "database"

                    messages_collection = component "Коллекция сообщений пользователей"{
                        tags "database"
                    }
                    posts_collection = component "Коллекция записей на стене пользователей"{
                        tags "database"
                    }
                }
            }

            rest_api_service -> user_database "Получение/обновление данных о пользователях"
            rest_api_service -> user_cache "Получение/обновление данных о пользователях"
            rest_api_service -> document_database "Получение/обновление данных о записях и сообщениях пользователейы"
            
            user -> rest_api_service "Отправка запросов и получение результатов"
        }

        prodution = deploymentEnvironment "Production" {
            aa = deploymentNode "User Server" {
                containerInstance social_network.rest_api_service
                instances 3
            }

            deploymentNode "databases" {
     
                deploymentNode "User Database Server" {
                    containerInstance social_network.user_database
                }

                deploymentNode "Document Database Server" {
                    containerInstance social_network.document_database
                    instances 3
                }

                deploymentNode "Cache Server" {
                    containerInstance social_network.user_cache
                }
            }
            
        }
    }

    views {
        systemContext social_network {
            autoLayout
            include *
        }

        container social_network {
            autoLayout
            include *
        }
        
        deployment social_network prodution {
            autoLayout
            include *
        }

        dynamic social_network "UC01" "Создание нового пользователя" {
            autoLayout
            user -> social_network.rest_api_service "Создать нового пользователя (POST /user)"
            social_network.rest_api_service -> social_network.user_database "Добавить запись о новом пользователе в базу данных"
            social_network.rest_api_service -> social_network.user_cache "Добавить запись о новом пользователе в кэш"
        }

        dynamic social_network "UC02" "Поиск пользователя по логину" {
            autoLayout
            user -> social_network.rest_api_service "Найти пользователя по логину (GET /user)"
            social_network.rest_api_service -> social_network.user_database "Получить запись о пользователе"
            social_network.rest_api_service -> social_network.user_cache "Получить запись о пользователе если она кэшированна"
        }

        dynamic social_network "UC03" "Поиск пользователя по маске имя и фамилии" {
            autoLayout
            user -> social_network.rest_api_service "Найти пользователя по маске имя и фамилии (GET /user/search)"
            social_network.rest_api_service -> social_network.user_database "Получить список записей подходящих пользователй"
        }

        dynamic social_network "UC04" "Добавление записи на стену" {
            autoLayout
            user -> social_network.rest_api_service "Добавление записи на стену (POST /user/{user_id}/posts)"
            social_network.rest_api_service -> social_network.document_database "Добавить новый документ в базу данных"
        }

        dynamic social_network "UC05" "Загрузка стены пользователя" {
            autoLayout
            user -> social_network.rest_api_service "Загрузка списка записей на стене пользователя (GET /user/{user_id}/posts)"
            social_network.rest_api_service -> social_network.document_database "Загрузить документы записей пользователя"
        }

        dynamic social_network "UC06" "Отправка сообщения пользователю" {
            autoLayout
            user -> social_network.rest_api_service "Отправка сообщения пользователю (POST /user/{user_id}/messages)"
            social_network.rest_api_service -> social_network.document_database "Добавить новый документ в базу данных"
        }

        dynamic social_network "UC07" "Получение списка сообщений для пользователя" {
            autoLayout
            user -> social_network.rest_api_service "Получение списка сообщений для пользователя (GET /user/{user_id}/messages)"
            social_network.rest_api_service -> social_network.document_database "Загрузить документы сообщений пользователя"
        }

        theme default

        styles {
            element "database" {
                shape cylinder
            }
        }
    }

}