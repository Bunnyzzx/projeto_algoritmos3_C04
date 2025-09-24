#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <algorithm>
#include <cmath>
#include <iomanip>
#include <limits>
#include <random>
#include <fstream>   // exportar CSV
#include <cstdlib>   // system()
using namespace std;

// ====== Constantes globais (exportação/Sheets) ======
const string CAMINHO_CSV = "/Users/caio/C04 - Algoritmos 3/projeto/dicionario/dicionario.csv";
const string CMD_ATUALIZAR_SHEETS =
    "python3 \"/Users/caio/C04 - Algoritmos 3/projeto/tabela_significados.py\"";

// ====== PROTÓTIPOS (importante para chamadas antecipadas) ======
void exportarCSV(const string& caminho);
void atualizarGoogleSheets();
void sincronizarSheets();

// ====== Estruturas ======
struct Palavra {
    string ficticio;                  // termo na língua fictícia
    vector<string> significadosPT;    // 1..N significados em português (nós do grafo)
    double x, y, z;                   // coordenadas
};

/// --- Banco de dados em memória ---
vector<Palavra> dicionario;

// Grafo bipartido: palavra -> significados e significado -> palavras
unordered_map<string, unordered_set<string>> adjPalavraParaSignificados;
unordered_map<string, unordered_set<string>> adjSignificadoParaPalavras;

// Índice rápido: fictício -> posição no vetor
unordered_map<string, int> idxPalavra;

/// Normaliza strings de significado (minúsculas, sem espaços extras)
static string norm(const string& s) {
    string t;
    for (char c : s) t.push_back(::tolower((unsigned char)c));
    // trim simples
    auto l = t.find_first_not_of(" \t\r\n");
    auto r = t.find_last_not_of(" \t\r\n");
    if (l==string::npos) return "";
    return t.substr(l, r-l+1);
}

void conectarNoGrafo(const string& fic, const vector<string>& sigs) {
    for (auto s : sigs) {
        s = norm(s);
        if (s.empty()) continue;
        adjPalavraParaSignificados[fic].insert(s);
        adjSignificadoParaPalavras[s].insert(fic);
    }
}

void descadastrarDoGrafo(const string& fic, const vector<string>& sigs) {
    for (auto s : sigs) {
        s = norm(s);
        if (!s.empty()) {
            adjPalavraParaSignificados[fic].erase(s);
            auto it = adjSignificadoParaPalavras.find(s);
            if (it != adjSignificadoParaPalavras.end()) {
                it->second.erase(fic);
                if (it->second.empty()) adjSignificadoParaPalavras.erase(it);
            }
        }
    }
    if (adjPalavraParaSignificados[fic].empty())
        adjPalavraParaSignificados.erase(fic);
}

/// --------- SEMENTE: 50 palavras + significados (algumas compartilham) ----------
void carregarSeed() {
    struct Seed { const char* pt; const char* fic; vector<string> extra; };
    vector<Seed> base = {
        {"fogo","Vharûn", {"chama"}},
        {"água","Léthar", {}},
        {"terra","Gorim", {}},
        {"ar","Sylhae", {}},
        {"luz","Elyth", {"claridade"}},
        {"sombra","Dravok", {"escuridão"}},
        {"vida","Almira", {}},
        {"morte","Kroth", {}},
        {"céu","Azyrel", {}},
        {"estrela","Thariel", {}},
        {"lua","Myrran", {}},
        {"sol","Zerath", {"luz","claridade"}}, // compartilha "luz"
        {"noite","Druval", {"escuridão"}},
        {"dia","Calyon", {"claridade"}},       // compartilha "claridade"
        {"pedra","Orvok", {}},
        {"montanha","Gravorn", {}},
        {"rio","Sylmar", {}},
        {"mar","Ocevrin", {}},
        {"floresta","Elwyn", {}},
        {"árvore","Tovaril", {}},
        {"folha","Feyra", {}},
        {"raiz","Drovah", {}},
        {"sangue","Rhogar", {}},
        {"espírito","Elyndar", {}},
        {"coração","Thyros", {}},
        {"olho","Veyrin", {}},
        {"mão","Drakar", {}},
        {"força","Khorval", {}},
        {"guerra","Zargrim", {}},
        {"paz","Mirath", {"tranquilidade"}},
        {"rei","Druvorn", {}},
        {"rainha","Selmira", {}},
        {"guerreiro","Thorgal", {}},
        {"mago","Arveth", {}},
        {"dragão","Vorash", {}},
        {"lobo","Fenryl", {}},
        {"cavalo","Grithor", {}},
        {"ouro","Aurvak", {}},
        {"prata","Sylven", {}},
        {"ferro","Grovik", {}},
        {"faca","Dareth", {}},
        {"espada","Khaldir", {}},
        {"escudo","Orvak", {}},
        {"portal","Zeythor", {"portão"}},
        {"fogo-fátuo","Lythir", {"chama"}},    // compartilha "chama"
        {"veneno","Drakthyl", {}},
        {"som","Orynel", {}},
        {"silêncio","Mythar", {}},
        {"tempo","Chronel", {}},
        {"eternidade","Veythar", {}},
    };

    // coordenadas “aleatórias” determinísticas para cada palavra
    mt19937_64 rng(42);
    uniform_real_distribution<double> dist(-1.0, 1.0);

    for (auto& s : base) {
        Palavra p;
        p.ficticio = s.fic;
        p.significadosPT = { norm(s.pt) };
        for (auto& e : s.extra) p.significadosPT.push_back(norm(e));
        p.x = dist(rng); p.y = dist(rng); p.z = dist(rng);

        idxPalavra[p.ficticio] = (int)dicionario.size();
        dicionario.push_back(p);
        conectarNoGrafo(p.ficticio, p.significadosPT);
    }
}

/// -------------------- Funcionalidades do MENU --------------------
void cadastrarPalavra() {
    Palavra p;
    cout << "Palavra (ficticia): ";
    cin >> ws;
    getline(cin, p.ficticio);
    if (idxPalavra.count(p.ficticio)) {
        cout << "Ja existe. Operacao cancelada.\n";
        return;
    }
    int n;
    cout << "Quantos significados em PT? ";
    cin >> n; cin.ignore(numeric_limits<streamsize>::max(), '\n');
    p.significadosPT.clear();
    for (int i=0;i<n;i++) {
        cout << "  Significado #" << (i+1) << ": ";
        string s; getline(cin, s);
        s = norm(s);
        if (!s.empty()) p.significadosPT.push_back(s);
    }
    cout << "Coordenadas x y z: ";
    cin >> p.x >> p.y >> p.z;

    idxPalavra[p.ficticio] = (int)dicionario.size();
    dicionario.push_back(p);
    conectarNoGrafo(p.ficticio, p.significadosPT);

    // Sincroniza automaticamente (CSV + Google Sheets)
    sincronizarSheets();

    cout << "Cadastrada!\n";
}

void listarSignificados() {
    cout << "Digite a palavra (ficticia): ";
    string fic; cin >> ws; getline(cin, fic);
    if (!idxPalavra.count(fic)) { cout << "Nao encontrada.\n"; return; }
    auto it = adjPalavraParaSignificados.find(fic);
    if (it == adjPalavraParaSignificados.end() || it->second.empty()) {
        cout << "Sem significados cadastrados.\n"; return;
    }
    cout << "Significados de " << fic << ": ";
    bool first=true;
    for (auto& s : it->second) {
        if (!first) cout << ", ";
        cout << s;
        first=false;
    }
    cout << "\n";
}

void listarSinonimos() {
    cout << "Digite a palavra (ficticia): ";
    string fic; cin >> ws; getline(cin, fic);
    if (!idxPalavra.count(fic)) { cout << "Nao encontrada.\n"; return; }

    unordered_set<string> sinonimos;
    for (auto& s : adjPalavraParaSignificados[fic]) {
        for (auto& w : adjSignificadoParaPalavras[s]) {
            if (w != fic) sinonimos.insert(w);
        }
    }
    if (sinonimos.empty()) { cout << "Nao ha sinonimos cadastrados.\n"; return; }

    vector<string> v(sinonimos.begin(), sinonimos.end());
    sort(v.begin(), v.end());
    cout << "Sinonimos de " << fic << ": ";
    for (size_t i=0;i<v.size();++i) {
        if (i) cout << ", ";
        cout << v[i];
    }
    cout << "\n";
}

void listarOrdemAlfabetica() {
    vector<string> v;
    v.reserve(dicionario.size());
    for (auto& p : dicionario) v.push_back(p.ficticio);
    sort(v.begin(), v.end());
    cout << "Palavras (A-Z):\n";
    for (auto& w : v) cout << " - " << w << "\n";
}

void listarPorTamanho() {
    vector<string> v;
    for (auto& p : dicionario) v.push_back(p.ficticio);
    stable_sort(v.begin(), v.end(), [](const string& a, const string& b){
        if (a.size() != b.size()) return a.size() < b.size();
        return a < b;
    });
    cout << "Palavras por tamanho:\n";
    for (auto& w : v) cout << " (" << w.size() << ") " << w << "\n";
}

void removerPalavra() {
    cout << "Digite a palavra a remover: ";
    string fic; cin >> ws; getline(cin, fic);
    if (!idxPalavra.count(fic)) { cout << "Nao encontrada.\n"; return; }
    int pos = idxPalavra[fic];
    descadastrarDoGrafo(fic, dicionario[pos].significadosPT);

    // remover do vetor mantendo estabilidade simples (swap com final)
    int last = (int)dicionario.size()-1;
    if (pos != last) {
        // atualizar índice da que vai para 'pos'
        idxPalavra[dicionario[last].ficticio] = pos;
        swap(dicionario[pos], dicionario[last]);
    }
    idxPalavra.erase(fic);
    dicionario.pop_back();

    // Sincroniza automaticamente (CSV + Google Sheets)
    sincronizarSheets();

    cout << "Removida.\n";
}

void calcularSimilaridade() {
    cout << "Digite duas palavras (ficticias):\n";
    string a,b; cin >> ws; getline(cin, a); getline(cin, b);
    if (!idxPalavra.count(a) || !idxPalavra.count(b)) {
        cout << "Uma das palavras nao existe.\n"; return;
    }
    auto &pa = dicionario[idxPalavra[a]];
    auto &pb = dicionario[idxPalavra[b]];
    auto dx = pa.x - pb.x, dy = pa.y - pb.y, dz = pa.z - pb.z;
    double dist = sqrt(dx*dx + dy*dy + dz*dz);
    cout << fixed << setprecision(4)
         << "Distancia Euclidiana entre " << a << " e " << b << " = " << dist << "\n";
}

// ====== Utilidades de UI ======
void clearScreen() {
    cout << "\x1b[2J\x1b[H";
    cout.flush();
}

void printHeader() {
    cout << "=====================================\n";
    cout << "     DICIONARIO — LINGUA FICTICIA    \n";
    cout << "=====================================\n\n";
}

void pauseEnter() {
    cout << "\nPressione ENTER para continuar...";
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
}

// ====== Exportação e sincronização ======
void exportarCSV(const string& caminho) {
    ofstream out(caminho);
    if (!out) {
        cout << "Falha ao abrir arquivo para escrita: " << caminho << "\n";
        return;
    }
    out << "Português;Baldurês\n";
    for (const auto& p : dicionario) {
        // Junta múltiplos significados em PT com " | "
        string pt;
        for (size_t i = 0; i < p.significadosPT.size(); ++i) {
            if (i) pt += " | ";
            pt += p.significadosPT[i];
        }
        // Aspas para proteger acentos e o separador
        out << '"' << pt << '"' << ';' << '"' << p.ficticio << '"' << "\n";
    }
    cout << "CSV exportado com sucesso para: " << caminho << "\n";
}

void atualizarGoogleSheets() {
    int rc = system(CMD_ATUALIZAR_SHEETS.c_str());
    if (rc != 0) {
        cout << "Aviso: falha ao executar script Python (codigo " << rc << ").\n";
    }
}

void sincronizarSheets() {
    exportarCSV(CAMINHO_CSV);
    atualizarGoogleSheets();
}

/// -------------------- MENU / MAIN --------------------
int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    carregarSeed();         // carrega as 50 palavras
    sincronizarSheets();    // opcional: já começa sincronizado

    int opcao;
    do {
        clearScreen();
        printHeader();

        cout << "==== MENU ====\n";
        cout << "1. Cadastrar palavra\n";
        cout << "2. Listar significados de uma palavra\n";
        cout << "3. Listar sinonimos de uma palavra\n";
        cout << "4. Listar palavras em ordem alfabetica\n";
        cout << "5. Listar palavras por ordem de tamanho\n";
        cout << "6. Remover palavra\n";
        cout << "7. Calcular similaridade entre duas palavras (coords)\n";
        cout << "0. Sair\n";
        cout << "Escolha uma opcao: ";

        if (!(cin >> opcao)) break;
        cin.ignore(numeric_limits<streamsize>::max(), '\n');

        clearScreen();
        printHeader();

        switch (opcao) {
            case 1: cadastrarPalavra(); break;
            case 2: listarSignificados(); break;
            case 3: listarSinonimos(); break;
            case 4: listarOrdemAlfabetica(); break;
            case 5: listarPorTamanho(); break;
            case 6: removerPalavra(); break;
            case 7: calcularSimilaridade(); break;
            case 0: cout << "Encerrando programa...\n"; break;
            default: cout << "Opcao invalida!\n"; break;
        }

        if (opcao != 0) {
            pauseEnter();
        }
    } while (opcao != 0);

    return 0;
}
