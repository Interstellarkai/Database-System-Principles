import psycopg2

# constants
DATABASE = "TPC-H"
HOST = "postgres"
PORT = "5432"
USER = "postgres"
PASSWORD = "password123"

# connect to database
print(f"[-] Connecting to database at {HOST}:{PORT}")
conn = psycopg2.connect(database=DATABASE, user=USER, password=PASSWORD, host=HOST, port=PORT)
print("[-] Database connected successfully!")

# select from small table
query = "SELECT * FROM region;"
print(f"[-] Executing test query: '{query}'")

cur = conn.cursor()
cur.execute(query)
rows = cur.fetchall()

# print results
print(f"[-] Query results:")
for row in rows:
    print(row)

# close connection
conn.close()
