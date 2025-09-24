import pandas as pd
import gspread
from gspread_dataframe import set_with_dataframe
from oauth2client.service_account import ServiceAccountCredentials
from pathlib import Path

# === Config ===
SHEET_URL = "https://docs.google.com/spreadsheets/d/1GU885LmlXJ6c--DNfq2FJJ7iOxy0yQ-n7TagEjSn3xw/edit?gid=0#gid=0"

# garante que o credentials.json é lido ao lado do .py
BASE = Path(__file__).resolve().parent
CRED_PATH = BASE / "credentials.json"

# === Dados ===
dados = {
    "Português": [
        "Fogo","Água","Terra","Ar","Luz","Sombra","Vida","Morte","Céu","Estrela",
        "Lua","Sol","Noite","Dia","Pedra","Montanha","Rio","Mar","Floresta","Árvore",
        "Folha","Raiz","Sangue","Espírito","Coração","Olho","Mão","Força","Guerra","Paz",
        "Rei","Rainha","Guerreiro","Mago","Dragão","Lobo","Cavalo","Ouro","Prata","Ferro",
        "Faca","Espada","Escudo","Portal","Fogo-fátuo","Veneno","Som","Silêncio","Tempo","Eternidade"
    ],
    "Baldurês": [
        "Vharûn","Léthar","Gorim","Sylhae","Elyth","Dravok","Almira","Kroth","Azyrel","Thariel",
        "Myrran","Zerath","Druval","Calyon","Orvok","Gravorn","Sylmar","Ocevrin","Elwyn","Tovaril",
        "Feyra","Drovah","Rhogar","Elyndar","Thyros","Veyrin","Drakar","Khorval","Zargrim","Mirath",
        "Druvorn","Selmira","Thorgal","Arveth","Vorash","Fenryl","Grithor","Aurvak","Sylven","Grovik",
        "Dareth","Khaldir","Orvak","Zeythor","Lythir","Drakthyl","Orynel","Mythar","Chronel","Veythar"
    ]
}
df = pd.DataFrame(dados)

# (opcional) índice começando em 1 para visual no Sheets
df.index = range(1, len(df) + 1)
df.index.name = "Nº"

# === Auth ===
scope = ["https://spreadsheets.google.com/feeds", "https://www.googleapis.com/auth/drive"]
creds = ServiceAccountCredentials.from_json_keyfile_name(str(CRED_PATH), scope)
client = gspread.authorize(creds)

# Abre pela URL e usa a primeira aba (sheet1)
sh = client.open_by_url(SHEET_URL)
ws = sh.sheet1

# Limpa e escreve
ws.clear()
set_with_dataframe(ws, df)

print("Planilha atualizada com sucesso!")
