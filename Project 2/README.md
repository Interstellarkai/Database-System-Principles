# Project 2 - PostgreSQL Query Plan Visualizer

## Introduction
This project aims to design and implement a user-friendly GUI application to retrieve and display information about the **Query Execution Plan (QEP)** and **Alternative Query Plans (AQP)** from a **PostgreSQL** database based on a user's SQL query. And annotate the corresponding SQL query to **explain how the underlying query processor executes ditterent components of the query.**

## Cloud
The web-app is deployed on AWS.
http://54.179.90.95

## Database Setup
1. Clone the repository.
    ```
    git clone https://github.com/Interstellarkai/Database-System-Principles
    ```

2. Change the working directory to the `database` folder.
    ```
    cd database
    ```

3. Download the `database-dump.sql` and `CSV` files from my onedrive, save it in the database folder in Step 2. 
    ```
    https://entuedu-my.sharepoint.com/:f:/g/personal/c200052_e_ntu_edu_sg/Elx5uRSw1nFHoec3Jyms2MAB56YOyNI_90vxadilnSQibQ?e=aExWQG
    ```

## Instantiate the containers
1. Use `docker-compose` to bring up the containers. During first creation, the database dump will be imported to the database automatically.
    ```
    cd ..
    docker-compose up -d
    ```

2. The database is accessible on port `5432`, pgadmin is accessible on port `8080`, and web-app is accesible on port `5000`.

## Contributors
The original repo is private and belongs to JunWei. This is a forked repo for public access.
- [Jun Wei](https://github.com/leejunweisg)
- [Kai Sheng](https://github.com/Interstellarkai)
- [JiaXin](https://github.com/Jiaxin0009)
- [Ying Sheng, Danny](https://github.com/dannyyys)
- [Zhu Zeyu](https://github.com/Zhu-Ze-Yu)