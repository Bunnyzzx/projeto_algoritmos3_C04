import pandas as pd

# Dados da tabela
dados = {
    "Português": [
        "Fogo", "Água", "Terra", "Ar", "Luz", "Sombra", "Vida", "Morte", "Céu", "Estrela",
        "Lua", "Sol", "Noite", "Dia", "Pedra", "Montanha", "Rio", "Mar", "Floresta", "Árvore",
        "Folha", "Raiz", "Sangue", "Espírito", "Coração", "Olho", "Mão", "Força", "Guerra", "Paz",
        "Rei", "Rainha", "Guerreiro", "Mago", "Dragão", "Lobo", "Cavalo", "Ouro", "Prata", "Ferro",
        "Faca", "Espada", "Escudo", "Portal", "Fogo-fátuo", "Veneno", "Som", "Silêncio", "Tempo", "Eternidade"
    ],
    "Baldurês": [
        "Vharûn", "Léthar", "Gorim", "Sylhae", "Elyth", "Dravok", "Almira", "Kroth", "Azyrel", "Thariel",
        "Myrran", "Zerath", "Druval", "Calyon", "Orvok", "Gravorn", "Sylmar", "Ocevrin", "Elwyn", "Tovaril",
        "Feyra", "Drovah", "Rhogar", "Elyndar", "Thyros", "Veyrin", "Drakar", "Khorval", "Zargrim", "Mirath",
        "Druvorn", "Selmira", "Thorgal", "Arveth", "Vorash", "Fenryl", "Grithor", "Aurvak", "Sylven", "Grovik",
        "Dareth", "Khaldir", "Orvak", "Zeythor", "Lythir", "Drakthyl", "Orynel", "Mythar", "Chronel", "Veythar"
    ]
}


# Criar DataFrame
df = pd.DataFrame(dados)

# Mostrar tabela
print(df.to_string())
