import sys
import pandas as pd
import gspread
from gspread_dataframe import set_with_dataframe
from oauth2client.service_account import ServiceAccountCredentials
from pathlib import Path

# === Config ===
SHEET_URL = "https://docs.google.com/spreadsheets/d/1GU885LmlXJ6c--DNfq2FJJ7iOxy0yQ-n7TagEjSn3xw/edit?gid=0#gid=0"

BASE = Path(__file__).resolve().parent
CRED_PATH = BASE / "credentials.json"
CSV_PATH = BASE / "dicionario" / "dicionario.csv"

# === Lê o CSV (separador ';') ===
try:
    df = pd.read_csv(CSV_PATH, sep=';', header=0, encoding="utf-8")
except FileNotFoundError:
    print(f"[erro] CSV não encontrado em: {CSV_PATH}")
    sys.exit(1)

# Índice visual começando em 1 (opcional)
df.index = range(1, len(df) + 1)
df.index.name = "Nº"

# === Auth ===
scope = ["https://spreadsheets.google.com/feeds",
         "https://www.googleapis.com/auth/drive"]
creds = ServiceAccountCredentials.from_json_keyfile_name(str(CRED_PATH), scope)
client = gspread.authorize(creds)

# Abre pela URL e usa a primeira aba (sheet1)
sh = client.open_by_url(SHEET_URL)
ws = sh.sheet1

# Limpa e escreve (inclui índice e rótulo do índice)
ws.clear()
set_with_dataframe(ws, df, include_index=True, include_index_label=True)

# (Opcional) congelar a primeira linha (cabeçalho)
ws.freeze(rows=1)

print("Planilha atualizada com sucesso a partir do CSV!")
