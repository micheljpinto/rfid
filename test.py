import mysql.connector

mydb = mysql.connector.connect(
  host="localhost",
  user="basico",
  password="@MIch1983*",
  database="Portaria"
)

mycursor = mydb.cursor()

mycursor.execute("SELECT user_Perm.cpf, user_Perm.nome,user_Perm.tag,user_Perm.status, user_Perm.key_safety_current FROM Portaria.user_Perm WHERE 1")

myresult = mycursor.fetchall()
print(myresult.__len__())

for x in myresult:
  print(x)
