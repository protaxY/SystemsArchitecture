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

            api_gateway = container "API GATEWAY" {
                description "API взаимодествия пользователя с социальной сетью"
            }

            user_service = container "User Service" {
                description "Сервис управления пользователями"
            }

            message_service = container "Message Service" {
                description "Сервис управления сообщениями"
            }

            post_service = container "Post Service" {
                description "Сервис управления записями на стенах пользователей"
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

            api_gateway -> user_service "Получение/обновление данных о пользователях"
            user_service -> user_database "Получение/обновление данных о пользователях"
            user_service -> user_cache "Получение/обновление данных о пользователях"

            api_gateway -> message_service "Получение/обновление данных о сообщениях пользователей"
            message_service -> document_database "Получение/обновление данных о сообщениях пользователей"

            api_gateway -> post_service "Получение/обновление данных о записях пользователей"
            post_service -> document_database "Получение/обновление данных о записях пользователей"

            user -> api_gateway "Отправка запросов и получение результатов"
        }

        prodution = deploymentEnvironment "Production" {
            deploymentNode "API GATEWAY" {
                containerInstance social_network.api_gateway
                instances 3
            }

            deploymentNode "User Server" {
                containerInstance social_network.user_service
                instances 3
            }

            deploymentNode "Message Server" {
                containerInstance social_network.message_service
                instances 3
            }

            deploymentNode "Post Server" {
                containerInstance social_network.post_service
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
            user -> social_network.api_gateway "Создать нового пользователя (POST /user)"
            social_network.api_gateway -> social_network.user_service "Создать нового пользователя"
            social_network.user_service -> social_network.user_database "Добавить запись о новом пользователе в базу данных"
            social_network.user_service -> social_network.user_cache "Добавить запись о новом пользователе в кэш"
        }

        dynamic social_network "UC02" "Поиск пользователя по логину" {
            autoLayout
            user -> social_network.api_gateway "Найти пользователя по логину (GET /user/{user_id})"
            social_network.api_gateway -> social_network.user_service "Получить запись о пользователе"
            social_network.user_service -> social_network.user_cache "Получить запись о пользователе если она кэшированна"
            social_network.user_service -> social_network.user_database "Получить запись о пользователе из базы данных"
            social_network.user_service -> social_network.user_cache "Добавить запись о пользователе в кэш"
        }

        dynamic social_network "UC03" "Поиск пользователя по маске имя и фамилии" {
            autoLayout
            user -> social_network.api_gateway "Найти пользователя по маске имя и фамилии (GET /user/search)"
            social_network.api_gateway -> social_network.user_service "Получить список записей подходящих пользователй"
            social_network.user_service -> social_network.user_database "Получить список записей подходящих пользователй из базы данных"
        }

        dynamic social_network "UC04" "Добавление записи на стену" {
            autoLayout
            user -> social_network.api_gateway "Добавить запись на стену (POST /user/{user_id}/posts)"
            social_network.api_gateway -> social_network.post_service "Добавить запись на стену"
            social_network.post_service -> social_network.document_database "Добавить документ о новой записи в базу данных"
        }

        dynamic social_network "UC05" "Загрузка стены пользователя" {
            autoLayout
            user -> social_network.api_gateway "Загрузить список записей на стене пользователя (GET /user/{user_id}/posts)"
            social_network.api_gateway -> social_network.post_service "Загрузить список записей на стене пользователя"
            social_network.post_service -> social_network.document_database "Загрузить документы записей пользователя"
        }

        dynamic social_network "UC06" "Отправка сообщения пользователю" {
            autoLayout
            user -> social_network.api_gateway "Отправить сообщение пользователю (POST /user/{user_id}/messages)"
            social_network.api_gateway -> social_network.message_service "Отправить сообщение пользователю"
            social_network.message_service -> social_network.document_database "Добавить документ нового сообщения в базу данных"
        }

        dynamic social_network "UC07" "Получение списка сообщений для пользователя" {
            autoLayout
            user -> social_network.api_gateway "Получить список сообщений пользователя (GET /user/{user_id}/messages)"
            social_network.api_gateway -> social_network.message_service "Получить список сообщений пользователя"
            social_network.message_service -> social_network.document_database "Загрузить документы сообщений пользователя"
        }

        theme default

        styles {
            element "database" {
                shape cylinder
            }
        }

        properties { 
            structurizr.tooltips true
        }
    }

}