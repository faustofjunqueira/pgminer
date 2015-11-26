#!/usr/bin/python3

from sys import exit
import os
import json
from subprocess import call

FILENAMEJSON = "obterTodasPosicoes"
DATABASE = "crawler_vdo"
TABLE = "log_posicao_onibus"
USER = "fausto"
LINK = "http://dadosabertos.rio.rj.gov.br/apiTransporte/apresentacao/rest/index.cfm/"

def removeFile(pathfile):
	if os.access(pathfile,os.F_OK):
		os.remove(pathfile)

def ReplaceFirstLetter(st,letter):
	out = letter
	out += st[1:]
	return out

def GetJsonWebServer():
	#if os.system("ping -c 1 "+LINK+FILENAMEJSON) == 0:
		call("wget "+LINK+FILENAMEJSON, shell=True)
	#else:
		#raise Exception("ServerError: Don't response from: "+LINK+FILENAMEJSON)	


def ReplaceLastLetter(st,letter):
	out = ""
	for i in st[:-1]:
		out += i
	out += letter
	return out

def GetColumns(Data):
	CatColumns = "("
	for column in Data['COLUMNS']:
		CatColumns += column + ","
	return ReplaceLastLetter(CatColumns,")")



def ExecuteSql(Data):
	SQLColumns = GetColumns(Data)
	removeFile("111476011jundaSQL.sql")
	with open("111476011jundaSQL.sql","w") as f:
		for data in Data['DATA']:
			SQLValues = ReplaceLastLetter(ReplaceFirstLetter(str(data),"("),")")
			f.write("INSERT INTO "+TABLE+SQLColumns+" VALUES "+SQLValues+";")
	call(["psql","-d", DATABASE,"-U",USER,"-w","-f","111476011jundaSQL.sql"]) #Insere no banco!
	removeFile("111476011jundaSQL.sql")

def ParseJSon(FileContent):
	Data = json.loads(FileContent)
	for i in range(len(Data["DATA"])):
		try:
			Data["DATA"][i][2] = str(int(Data["DATA"][i][2]))
		except ValueError:
			#Aqui aparece quando nao tem linha
			continue
	
	return Data

def main():
	removeFile(FILENAMEJSON)
	try:	
		GetJsonWebServer()
	except Exception as error:
		print(error)
		exit()

	with open(FILENAMEJSON, "r") as f:	
		Data = ParseJSon(f.read())		
		ExecuteSql(Data)
	os.remove(FILENAMEJSON)
	print(str(len(Data))+" data were entered in junda "+TABLE+" database "+DATABASE)
main()
