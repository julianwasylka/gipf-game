#include <iostream>
#include <iomanip>
#include <vector>
#include <map>
#include <string>

#include <utility>
#include <algorithm>

using namespace std;

static const char GRACZ_W = 'W';
static const char GRACZ_B = 'B';

static const char POLE_START = '@';
static const char POLE_PUSTE = '_';
static const char DROGA = ' ';
static const char TLO = '.';

static const int START_SCIEZKI = -1;
static const int KONIEC_SCIEZKI = 1;

enum Stan
{
    IN_PROGRESS,
    WHITE_WIN,
    BLACK_WIN,
    DEAD_LOCK,
    BAD_MOVE
};

enum Cmd
{
    NONE,
    LOAD_GAME_BOARD,
    PRINT_GAME_BOARD,
    DO_MOVE,
    PRINT_GAME_STATE,
    GEN_ALL_POS_MOV,
    GEN_ALL_POS_MOV_NUM,
    WINNING_SEQUENCE_EXIST,
    SOLVE_GAME_STATE,
    QUIT
};

Cmd zrobKomende(string& str)
{
    if (str == "LOAD_GAME_BOARD") return LOAD_GAME_BOARD;
    else if (str == "PRINT_GAME_BOARD") return PRINT_GAME_BOARD;
    else if (str == "DO_MOVE") return DO_MOVE;
    else if (str == "PRINT_GAME_STATE") return PRINT_GAME_STATE;
    else if (str == "GEN_ALL_POS_MOV") return GEN_ALL_POS_MOV;
    else if (str == "GEN_ALL_POS_MOV_NUM") return GEN_ALL_POS_MOV_NUM;
    else if (str == "WINNING_SEQUENCE_EXIST") return WINNING_SEQUENCE_EXIST;
    else if (str == "SOLVE_GAME_STATE") return SOLVE_GAME_STATE;
    else if (str == "QUIT") return QUIT;
    return NONE;
}

class Gracz
{
private:
    int calosc = 0;
    int rezerwa = 0;
public:
    int getCalosc() const;
    void setCalosc(int c);
    int getRezerwa();
    void setRezerwa(int r);
    Gracz(int calosc = 0, int rezerwa = 0);
    Gracz(Gracz const& old);
    ~Gracz();
    Gracz& operator=(const Gracz& other);
};

Gracz::Gracz(int calosc, int rezerwa) : calosc(calosc), rezerwa(rezerwa) {}

Gracz::Gracz(Gracz const& old) : Gracz(old.calosc, old.rezerwa) {}

Gracz& Gracz::operator=(const Gracz& other)
{
    if (this == &other)
    {
        return *this; 
    }

    calosc = other.calosc;
    rezerwa = other.rezerwa;

    return *this;
}

int Gracz::getCalosc() const
{
    return calosc;
}
void Gracz::setCalosc(int c)
{
    calosc = c;
}

int Gracz::getRezerwa()
{
    return rezerwa;
}
void Gracz::setRezerwa(int r)
{
    rezerwa = r;
}

Gracz::~Gracz() {}

class Plansza
{
private:
    int S = 0;
    int K = 0;
    bool pusta = false;
    Gracz W;
    Gracz B;
    char biezacyGracz = GRACZ_W;
    Stan stan = IN_PROGRESS;
    string zlyRuch;
    vector<vector<char>> gra;
    map<string, string> xyDoIndex;  // "x,y" -> a1
    map<string, string> indexDoXy;  // a1 -> "x,y"
    map<string, int> startKoniecSciezki; // a1 -> START_SCIEZKI, b2 -> KONIEC_SCIEZKI
    map<string, vector<pair<int, int>>> mozliwe_sciezki; //  "x,y-x,y" -> [x,y],[x,y]

    void dodajDoSciezek(vector<pair<int, int>>& linia);
    void zrobSciezki();
    bool wolneMiejsce(vector<pair<int, int>>& sciezka);
    void zmienGracza();
    int czyPowtarzajaSie(string& tekst, char* gracz);
    void usunJedenPionek(string const& key, char c, char gracz, int i);
    int usunPionki(string const& key, string& text, char gracz, int idx);
    void generujIndexPlanszy();
    void sprawdzStan();
    void wyczysc();
public:
    Plansza();
    Plansza(Plansza const& old);
    Plansza& operator=(const Plansza& other);
    bool getPusta();
    void rysuj();
    bool wczytaj();
    void nowaPlansza(vector<vector<char>>& old);
    bool ruch();
    void wykonajRuch(vector<pair<int, int>>& sciezka, char gracz);
    void stanGry();
    vector<pair<string, string>> generujMozliwePosuniecia();
    void wypiszMozliwePosuniecia(bool tylkoIlosc);
    int zbijPionki(char gracz);
    ~Plansza();
};

Plansza::Plansza() {}

Plansza& Plansza::operator=(const Plansza& other)
{
    if (this == &other) 
    {
        return *this;
    }

    S = other.S;
    K = other.K;
    pusta = other.pusta;
    W = other.W;
    B = other.B;
    biezacyGracz = other.biezacyGracz;
    stan = other.stan;
    zlyRuch = other.zlyRuch;
    gra = other.gra;
    xyDoIndex = other.xyDoIndex;
    indexDoXy = other.indexDoXy;
    startKoniecSciezki = other.startKoniecSciezki;
    mozliwe_sciezki = other.mozliwe_sciezki;

    return *this;
}

Plansza::Plansza(Plansza const& old) :
    S(old.S), K(old.K), W(old.W), B(old.B), biezacyGracz(old.biezacyGracz), stan(old.stan), zlyRuch(old.zlyRuch),
    gra(old.gra), xyDoIndex(old.xyDoIndex), indexDoXy(old.indexDoXy),
    startKoniecSciezki(old.startKoniecSciezki), mozliwe_sciezki(old.mozliwe_sciezki) {}

bool Plansza::getPusta()
{
    return pusta;
}

bool Plansza::wczytaj()
{
    int _s = 0, _k = 0;
    int _gw = 0, _gb = 0;
    int gw = 0, gb = 0;
    int rgw = 0, rgb = 0;
    char g = ' ';
    string row;
    pusta = false;
    cin >> _s >> _k >> _gw >> _gb;

    gw = _gw;
    gb = _gb;

    cin >> rgw >> rgb >> g;

    cin.ignore(); // przed getline obcina poprzedni koniec linii
    bool zlaDlugoscWiersza = false;
    vector<vector<char>> _gra;
    _gra.resize(2 * _s - 1, vector<char>(4 * _s - 3, ' '));

    string line;
    int i = -1;
    while (getline(cin, line))
    {
        if (line.empty() || cin.eof())
        {
            break;
        }

        int j = -1;
        int k = 0; // ilosc znakow w rzedzie
        i++;
        for (char c : line)
        {
            j++;
            if (c != ' ')
            {
                k++;
                _gra[i][j] = c;
                if (c == GRACZ_W) --gw;
                else if (c == GRACZ_B) --gb;
            }
        }

        int iRow = i < _s ? _s + i : 2 * _s - 1 - ((i + 1) % _s);
        if (iRow != k)
        {
            zlaDlugoscWiersza = true;
        }
    }

    if (zlaDlugoscWiersza)
    {
        cout << "WRONG_BOARD_ROW_LENGTH" << endl;
        pusta = true;
        return false;
    }

    if (gw < rgw) {
        cout << "WRONG_WHITE_PAWNS_NUMBER" << endl;
        pusta = true;
        return false;
    }

    if (gb < rgb) {
        cout << "WRONG_BLACK_PAWNS_NUMBER" << endl;
        pusta = true;
        return false;
    }

    S = _s;
    K = _k;
    W.setCalosc(_gw);
    W.setRezerwa(rgw);
    B.setCalosc(_gb);
    B.setRezerwa(rgb);
    biezacyGracz = g;

    nowaPlansza(_gra);

    char gracz = ' ';
    int licznik = 0;
    string tekst;
    for (auto& linia : mozliwe_sciezki)
    {
        tekst.clear();
        for (auto& xy : linia.second)
        {
            tekst.push_back(gra[xy.second][xy.first]);
        }
        int idx = czyPowtarzajaSie(tekst, &gracz);
        if (idx >= 0)
        {
            licznik++;
        }
    }

    if (licznik > 0)
    {
        licznik /= 2;
        cout << "ERROR_FOUND_" << licznik << "_ROW"
            << (licznik == 1 ? "" : "S") << "_OF_LENGTH_K" << endl;
        return false;
    }

    cout << "BOARD_STATE_OK" << endl;

    for (vector<char> _v : _gra)
        _v.clear();
    _gra.clear();

    return true;
}

void Plansza::wyczysc()
{
    xyDoIndex.clear();
    indexDoXy.clear();
    startKoniecSciezki.clear();
    mozliwe_sciezki.clear();

    for (int i = 0; i < (int)gra.size(); i++)
    {
        gra[i].clear();
    }
    gra.clear();
}

void Plansza::nowaPlansza(vector<vector<char>>& old)
{
    wyczysc();
    gra.resize(2 * S - 1 + 2, vector<char>(4 * S - 3 + 4, TLO));

    int ySize = gra.size();
    int xSize = gra[0].size();

    for (int y = 0; y < ySize; y++)
    {
        int znak = POLE_START;
        bool empty = false;
        for (int x = (y < S ? S - y % S : y - S); x < (y < S ? xSize - S + y % S : xSize - y + S); x++)
        {
            if (empty)
            {
                empty = false;
                if ((y != 0) && (y != ySize - 1))
                {
                    gra[y][x] = DROGA;
                }
            }
            else
            {
                if ((y == 0) || (y == ySize - 1) ||
                    (x == (y < S ? xSize - S + y % S - 1 : xSize - y + S - 1)))
                {
                    znak = POLE_START;
                }
                if (znak == POLE_PUSTE && (old[y - 1][x - 2] == GRACZ_W || old[y - 1][x - 2] == GRACZ_B))
                {
                    znak = old[y - 1][x - 2];
                }

                gra[y][x] = (char)znak;
                empty = true;
                znak = POLE_PUSTE;
            }
        }
    }

    generujIndexPlanszy();
    zrobSciezki();
}

void Plansza::generujIndexPlanszy()
{
    /* na skos z dolu w prawo
    przyklad:
           S=2      y
      a3 b4 c5      0
     a2 b3 c4 d4    1
    a1 b2 c3 d3 e3  2
      b1 c2 d2 e2   3
        c1 d1 e1    4
    0 1 2  4  6  <- x
    */
    string key;
    string val;

    for (int i = 0; i <= 2 * S; i++)
    {
        int x = i <= S ? i : i + i - S;
        int y = i < S ? S + i : 2 * S;
        int kroki = i <= S ? S + i + 1 : 3 * S + 1 - i;
        for (int j = 1; j <= kroki; j++)
        {
            key.clear();
            val.clear();
            key = to_string(x) + ',' + to_string(y);
            val.push_back(char('a' + i));
            val.append(to_string(j));
            xyDoIndex.emplace(key, val);
            indexDoXy.emplace(val, key);
            x++;
            y--;
        }
    }
}

void Plansza::rysuj()
{
    if (gra.size() == 0) return;

    cout << S << ' ' << K << ' ' << W.getCalosc() << ' ' << B.getCalosc() << endl;
    cout << W.getRezerwa() << ' ' << B.getRezerwa() << ' ' << biezacyGracz << endl;

    for (int y = 1; y < (int)gra.size() - 1; y++)
    {
        for (int x = 2; x < (int)(gra[y].size() - 2); x++)
        {
            if (gra[y][x] == GRACZ_B)
                cout << 'B';
            else if (gra[y][x] == GRACZ_W)
                cout << 'W';
            else if (gra[y][x] == POLE_PUSTE)
                cout << '_';
            else
                cout << ' ';
        }
        cout << endl;
    }
}

bool Plansza::wolneMiejsce(vector<pair<int, int>>& sciezka)
{
    for (auto& xy : sciezka)
    {
        if (gra[xy.second][xy.first] == POLE_PUSTE)
        {
            return true;
        }
    }
    return false;
}

void Plansza::wykonajRuch(vector<pair<int, int>>& sciezka, char gracz)
{
    // pionek z rezerwy na plansze
    if (gracz == GRACZ_W) W.setRezerwa(W.getRezerwa() - 1);
    else B.setRezerwa(B.getRezerwa() - 1);

    // przesuwanie pionkow po sciezce
    char poprzedniZnak = gracz;
    for (auto xy : sciezka)
    {
        char znak = gra[xy.second][xy.first];
        if (znak == POLE_PUSTE)
        {
            gra[xy.second][xy.first] = poprzedniZnak;
            return;
        }
        gra[xy.second][xy.first] = poprzedniZnak;
        poprzedniZnak = znak;
    }
}

void Plansza::zmienGracza()
{
    if (biezacyGracz == GRACZ_B)
    {
        biezacyGracz = GRACZ_W;
    }
    else
    {
        biezacyGracz = GRACZ_B;
    }
}

bool Plansza::ruch()
{
    char c;
    string start;
    string koniec;

    while ((c = (char)getchar()) != '-')
    {
        if (c != ' ')
            start.push_back(c);
    }
    while ((c = (char)getchar()) != '\n')
    {
        if (c != ' ')
            koniec.push_back(c);
        else break;
    }

    if (indexDoXy.find(start) == indexDoXy.end())
    {
        cout << "BAD_MOVE_" << start << "_IS_WRONG_INDEX" << endl;
        stan = BAD_MOVE;
        zlyRuch.push_back(biezacyGracz);
        zlyRuch.append(" " + start + '-' + koniec);
        return false;
    }

    if (indexDoXy.find(koniec) == indexDoXy.end())
    {
        cout << "BAD_MOVE_" << koniec << "_IS_WRONG_INDEX" << endl;
        stan = BAD_MOVE;
        zlyRuch.push_back(biezacyGracz);
        zlyRuch.append(" " + start + '-' + koniec);
        return false;
    }

    if (startKoniecSciezki.at(start) != START_SCIEZKI)
    {
        cout << "BAD_MOVE_" << start << "_IS_WRONG_STARTING_FIELD" << endl;
        stan = BAD_MOVE;
        zlyRuch.push_back(biezacyGracz);
        zlyRuch.append(" " + start + '-' + koniec);
        return false;
    }

    if (startKoniecSciezki.at(koniec) != KONIEC_SCIEZKI)
    {
        cout << "BAD_MOVE_" << koniec << "_IS_WRONG_DESTINATION_FIELD" << endl;
        stan = BAD_MOVE;
        zlyRuch.push_back(biezacyGracz);
        zlyRuch.append(" " + start + '-' + koniec);
        return false;
    }

    string kluczSciezek = indexDoXy.at(start) + '-' + indexDoXy.at(koniec);
    if (mozliwe_sciezki.find(kluczSciezek) == mozliwe_sciezki.end())
    {
        stan = BAD_MOVE;
        zlyRuch.push_back(biezacyGracz);
        zlyRuch.append(" " + start + '-' + koniec);
        cout << "UNKNOWN_MOVE_DIRECTION" << endl;
        return false;
    }

    vector<pair<int, int>> sciezka = mozliwe_sciezki.at(kluczSciezek);
    if (wolneMiejsce(sciezka))
    {
        zlyRuch.clear();
        stan = IN_PROGRESS;
        wykonajRuch(sciezka, biezacyGracz);
        zbijPionki(biezacyGracz);
        zmienGracza();
        sprawdzStan();

        cout << "MOVE_COMMITTED" << endl;
        return true;
    }
    else
    {
        cout << "BAD_MOVE_ROW_IS_FULL" << endl;
    }

    return false;
}

void Plansza::dodajDoSciezek(vector<pair<int, int>>& linia)
{
    string lewyStart = to_string(linia.front().first) + ',' + to_string(linia.front().second);
    linia.erase(linia.begin());
    string lewyKoniec = to_string(linia.front().first) + ',' + to_string(linia.front().second);

    string prawyStart = to_string(linia.back().first) + ',' + to_string(linia.back().second);
    linia.pop_back();
    string prawyKoniec = to_string(linia.back().first) + ',' + to_string(linia.back().second);

    mozliwe_sciezki.emplace(lewyStart + '-' + lewyKoniec, linia);
    startKoniecSciezki.emplace(xyDoIndex.at(lewyStart), START_SCIEZKI);
    startKoniecSciezki.emplace(xyDoIndex.at(lewyKoniec), KONIEC_SCIEZKI);

    reverse(linia.begin(), linia.end());
    mozliwe_sciezki.emplace(prawyStart + '-' + prawyKoniec, linia);
    startKoniecSciezki.emplace(xyDoIndex.at(prawyStart), START_SCIEZKI);
    startKoniecSciezki.emplace(xyDoIndex.at(prawyKoniec), KONIEC_SCIEZKI);
}

void Plansza::zrobSciezki()
{
    // sciezka x,y zaczyna sie i konczy punktem startowym
    int ySize = gra.size();
    int xSize = gra[0].size();
    // poziomo
    for (int y = 1; y < ySize - 1; y++)
    {
        vector<pair<int, int>> linia;
        for (int x = (y < S ? S - y % S : y - S); x < (y < S ? xSize - S + y % S : xSize - y + S); x += 2)
        {
            linia.push_back(make_pair(x, y));
        }
        dodajDoSciezek(linia);
    }
    // skos z gory w prawo
    for (int xj = 1; xj < 2 * S; xj++)
    {
        int x = xj <= S ? xj : xj + xj % S;
        int y = xj < S ? S - xj : 0;
        vector<pair<int, int>> linia;
        do
        {
            linia.push_back(make_pair(x, y));
            x++;
            y++;
        } while (gra[y][x] != POLE_START);
        linia.push_back(make_pair(x, y));
        dodajDoSciezek(linia);
    }
    // skos z dolu w prawo
    for (int xj = 1; xj < 2 * S; xj++)
    {
        int x = xj <= S ? xj : xj + xj % S;
        int y = xj < S ? S + xj : 2 * S;
        vector<pair<int, int>> linia;
        do
        {
            linia.push_back(make_pair(x, y));
            x++;
            y--;
        } while (gra[y][x] != POLE_START);
        linia.push_back(make_pair(x, y));
        dodajDoSciezek(linia);
    }
}

int Plansza::zbijPionki(char gracz)
{
    int zbitePionki = 0;
    char kolorGraczaZKPionkami = ' ';
    string tekst;
    for (auto& linia : mozliwe_sciezki)
    {
        tekst.clear();
        for (auto& xy : linia.second)
        {
            tekst.push_back(gra[xy.second][xy.first]);
        }
        int idx = czyPowtarzajaSie(tekst, &kolorGraczaZKPionkami);
        if (idx >= 0)
        {
            zbitePionki = usunPionki(linia.first, tekst, kolorGraczaZKPionkami, idx);
        }
    }
    // zwracaj tylko ile zbitych pionkow przeciwnika
    if (gracz == kolorGraczaZKPionkami)
    {
        return zbitePionki;
    }
    return 0;
}

void Plansza::usunJedenPionek(string const& key, char c, char gracz, int i)
{
    if (c != GRACZ_W && c != GRACZ_B) return;

    if (gracz == GRACZ_W)
    {
        if (c == GRACZ_W)
        {
            W.setRezerwa(W.getRezerwa() + 1);
        }
    }

    if (gracz == GRACZ_B)
    {
        if (c == GRACZ_B)
        {
            B.setRezerwa(B.getRezerwa() + 1);
        }
    }
    int x = mozliwe_sciezki.at(key)[i].first;
    int y = mozliwe_sciezki.at(key)[i].second;
    gra[y][x] = POLE_PUSTE;
}

int Plansza::usunPionki(string const& key, string& text, char gracz, int idx)
{
    int licznik = 0;
    for (int i = idx; i >= 0 && text[i] != POLE_PUSTE; i--)
    {
        usunJedenPionek(key, text[i], gracz, i);
        licznik++;
    }

    for (int i = idx + 1; i < (int)text.size() && text[i] != POLE_PUSTE; i++)
    {
        usunJedenPionek(key, text[i], gracz, i);
        licznik++;
    }

    return licznik;
}

int Plansza::czyPowtarzajaSie(string& tekst, char* gracz)
{
    int licznik = 1;

    for (size_t i = 1; i < tekst.length(); i++)
    {
        if ((tekst[i] == GRACZ_W || tekst[i] == GRACZ_B) && tekst[i] == tekst[i - 1])
        {
            licznik++;
            if (licznik == K)
            {
                *gracz = tekst[i];
                return i;
            }
        }
        else
        {
            licznik = 1;
        }
    }

    return -1;
}

void Plansza::sprawdzStan()
{
    if (B.getRezerwa() <= 0)
    {
        stan = WHITE_WIN;
    }
    else if (W.getRezerwa() <= 0)
    {
        stan = BLACK_WIN;
    }
    else if (generujMozliwePosuniecia().size() == 0)
    {
        stan = DEAD_LOCK;
    }
    else if (zlyRuch.size() > 0)
    {
        stan = BAD_MOVE;
    }
    else
    {
        stan = IN_PROGRESS;
    }
}

vector<pair<string, string>> Plansza::generujMozliwePosuniecia()
{
    vector<pair<string, string>> mozliwePosuniecia;
    for (auto& linia : mozliwe_sciezki)
    {
        if (wolneMiejsce(linia.second))
        {
            string tekst = linia.first;
            size_t pozycja = tekst.find('-');
            string poczatek = tekst.substr(0, pozycja);
            string koniec = tekst.substr(pozycja + 1);
            mozliwePosuniecia.push_back(make_pair(poczatek, koniec));
        }
    }
    return mozliwePosuniecia;
}

void Plansza::wypiszMozliwePosuniecia(bool tylkoIlosc)
{
    vector<pair<string, string>> m = generujMozliwePosuniecia();
    if (tylkoIlosc)
    {
        cout << m.size() << "_UNIQUE_MOVES" << endl;
    }
    else
    {
        for (auto& r : m)
        {
            cout << xyDoIndex.at(r.first) << "-" << xyDoIndex.at(r.second) << endl;
        }
    }
}

void Plansza::stanGry()
{
    sprawdzStan();
    switch (stan)
    {
    case IN_PROGRESS:
        cout << "in_progress" << endl;
        break;

    case WHITE_WIN:
        cout << "white_win" << endl;
        break;

    case BLACK_WIN:
        cout << "black_win" << endl;
        break;

    case DEAD_LOCK:
        cout << "dead_lock" << ' ' << biezacyGracz << endl;
        break;

    case BAD_MOVE:
        cout << "bad_move" << zlyRuch << endl;
        break;

    default:
        break;
    }
}

Plansza::~Plansza()
{
    wyczysc();
}

class Komenda
{
private:
    Cmd cmd;
public:
    Komenda();
    void czytaj(Plansza* p);
    ~Komenda();
};

Komenda::Komenda() : cmd(NONE) {}

void Komenda::czytaj(Plansza* p)
{
    string polecenie;

    while (!cin.eof())
    {
        polecenie.clear();
        cin >> polecenie;

        if (polecenie.empty()) continue;
        cmd = zrobKomende(polecenie);
        // p->rysujZOznaczeniami();
        switch (cmd)
        {
        case QUIT:
            break;

        case LOAD_GAME_BOARD:
            p->wczytaj();
            break;

        case PRINT_GAME_BOARD:
            if (p->getPusta())
            {
                cout << "EMPTY_BOARD" << endl;
                break;
             }
            p->rysuj();
            break;

        case DO_MOVE:
            p->ruch();
            break;

        case PRINT_GAME_STATE:
            p->stanGry();
            break;

        case GEN_ALL_POS_MOV:
            p->wypiszMozliwePosuniecia(false);
            break;

        case GEN_ALL_POS_MOV_NUM:
            p->wypiszMozliwePosuniecia(true);
            break;

        default:
            break;
        }
    }
}

Komenda::~Komenda() {}

int main()
{
    Plansza* p = new Plansza();
    Komenda* c = new Komenda();

    c->czytaj(p);

    delete p;
    delete c;

    return 0;
}