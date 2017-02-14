#include <bits/stdc++.h>

#include <glpk.h>

using namespace std;

#define FORE(it,c)  for(__typeof((c).begin()) it = (c).begin(); it != (c).end(); ++it)
#define FOR(i,a,b)  for(int i=(a);i<(b);++i)
#define REP(i,a)    FOR(i,0,a)
#define ZERO(m)    memset(m,0,sizeof(m))
#define ALL(x)      x.begin(),x.end()
#define PB          push_back
#define S          size()
#define LL          long long
#define ULL        unsigned long long
#define LD          long double
#define MP          make_pair
#define X          first
#define Y          second
#define VC          vector
#define PII        pair <int, int>
#define VI          VC < int >
#define VVI        VC < VI >
#define VD          VC < double >
#define VVD        VC < VD >
#define VS          VC < string >
#define DB(a)      cerr << #a << ": " << (a) << endl;

template<class T> void print(VC < T > v) {cerr << "[";if (v.S) cerr << v[0];FOR(i, 1, v.S) cerr << ", " << v[i];cerr << "]\n";}
template<class T> string i2s(T x) {ostringstream o; o << x; return o.str(); }
VS splt(string s, char c = ' ') {VS all; int p = 0, np; while (np = s.find(c, p), np >= 0) {if (np != p) all.PB(s.substr(p, np - p)); p = np + 1;} if (p < s.S) all.PB(s.substr(p)); return all;}

const int MAX_DIM = 1024;

int bmp[MAX_DIM][MAX_DIM];
int cur[MAX_DIM][MAX_DIM];

int n, m;

int greedy() {
	
}

glp_prob *lp;

void lpcallback(glp_tree *tree, void *info) {
	if (glp_ios_reason(tree) == GLP_IBINGO) {
		DB(glp_mip_obj_val(lp));
	}
}

int dr[] = {0, 1, 1, 1};
int dc[] = {1, -1, 0, 1};

bool ok(int r, int c) {
	return r >= 0 && r < n && c >= 0 && c < m && cur[r][c];
}

VS operations;
VI pixels[MAX_DIM][MAX_DIM];
VS lpsolve() {
	REP(r0, n) REP(c0, m) if (ok(r0, c0)) {
		int size = 1;
		while (true) {
			bool good = true;
			FOR(i, -size, size+1) good &= ok(r0-size,c0+i);
			FOR(i, -size, size+1) good &= ok(r0+size,c0+i);
			FOR(i, -size, size+1) good &= ok(r0+i,c0-size);
			FOR(i, -size, size+1) good &= ok(r0+i,c0+size);
			if (!good) break;
			size++;
		}
		size--;
		// if (size == 0) continue;
		FOR(r, r0-size, r0+size+1) FOR(c, c0-size, c0+size+1)
			pixels[r][c].PB(operations.S);
		char s[1000];
		sprintf(s, "PAINT_SQUARE %d %d %d", r0, c0, size);
		operations.PB(s);
	}
	DB(operations.S);
	
	REP(r0, n) REP(c0, m) if (ok(r0, c0) && !ok(r0-1, c0)) {
		int len = 0;
		pixels[r0][c0].PB(operations.S);
		while (ok(r0+len+1, c0)) {
			pixels[r0+len+1][c0].PB(operations.S);
			len++;
		}
		// if (len == 0) continue;
		char s[1000];
		sprintf(s, "PAINT_LINE %d %d %d %d", r0, c0, r0+len, c0);
		operations.PB(s);
	}
	DB(operations.S);
	
	REP(r0, n) REP(c0, m) if (ok(r0, c0) && !ok(r0, c0-1)) {
		int len = 0;
		pixels[r0][c0].PB(operations.S);
		while (ok(r0, c0+len+1)) {
			pixels[r0][c0+len+1].PB(operations.S);
			len++;
		}
		// if (len == 0) continue;
		char s[1000];
		sprintf(s, "PAINT_LINE %d %d %d %d", r0, c0, r0, c0+len);
		operations.PB(s);
	}
	
	DB(operations.S);
	
	VI lpr, lpc;
	VD lpv;
	
	int rows = 0;
	lpr.PB(0);
	lpc.PB(0);
	lpv.PB(0);
	REP(r, n) REP(c, m) if (cur[r][c]) {
		REP(i, pixels[r][c].S) {
			lpr.PB(rows + 1);
			lpc.PB(pixels[r][c][i] + 1);
			lpv.PB(1.0);
		}
		rows++;
		
	}

	glp_term_out(GLP_OFF);
	
	lp = glp_create_prob();
	glp_set_prob_name(lp, "sample");
	glp_set_obj_dir(lp, GLP_MIN);
	
	glp_add_rows(lp, rows);
	REP(i, rows) glp_set_row_bnds(lp, i + 1, GLP_LO, 1.0, 0.0);
	
	glp_add_cols(lp, operations.S);
	REP(i, operations.S) {
		glp_set_col_bnds(lp, i + 1, GLP_DB, 0.0, 1.0);
		glp_set_obj_coef(lp, i + 1, 1.0);
		glp_set_col_kind(lp, i + 1, GLP_BV);
	}
	
	glp_load_matrix(lp, lpr.S - 1, &lpr[0], &lpc[0], &lpv[0]);
	
	glp_iocp parm;
	glp_init_iocp(&parm);
	parm.presolve = GLP_ON;
	parm.cb_func = lpcallback;
	parm.cb_info = NULL;
	glp_intopt(lp, &parm);
	
	DB(glp_mip_obj_val(lp));
	
	VS rv;
	REP(i, operations.S) {
		if (glp_mip_col_val(lp, i + 1)) rv.PB(operations[i]);
	}
	
	glp_delete_prob(lp);
	
	return rv;
}

bool validate(VS v) {
	memcpy(cur, bmp, sizeof(int)*MAX_DIM*n);
	
	bool error = false;
	
	REP(i, v.S) {
		char s[100];
		int r0 = -1, r1 = -1, c0 = -1, c1 = -1;
		sscanf(v[i].c_str(), "%s %d %d %d %d", s, &r0, &c0, &r1, &c1);
		
		if (c1 == -1) {
			int size = r1;
			c1 = c0 + size;
			r1 = r0 + size;
			r0 = r0 - size;
			c0 = c0 - size;
		}
		
		FOR(r, r0, r1+1) FOR(c, c0, c1+1) {
			if (bmp[r][c] == 0) {
				error = true;
				cerr << "Error in: " << v[i] << endl;
			}
			cur[r][c] = 0;			
		}
	}
	
	int rem = 0;
	REP(r, n) REP(c, m) rem += cur[r][c];
	
	if (rem) {
		REP(r, n) {
			REP(c, m) cout << (char)(cur[r][c] ? '#' : '.');
			cout << endl;
		}
		DB(rem);
	}
	
	return rem == 0 || error;
}

int main() {
	cin >> n >> m;
	REP(i, n) {
		string s;
		cin >> s;
		REP(j, m) bmp[i][j] = s[j] == '#';
	}
	
	memcpy(cur, bmp, sizeof(int)*MAX_DIM*n);
	VS clears;
	// FOR(r, 1, n - 1) FOR(c, 1, m - 1) if (!ok(r, c) && ok(r - 1, c) && ok(r + 1, c) && ok(r, c - 1) && ok(r, c + 1)) {
		// bmp[r][c] = 1;
		// char s[100];
		// sprintf(s, "CLEAR %d %d", r + 1, c + 1);
		// clears.PB(s);
	// }
	DB(clears.S);
	memcpy(cur, bmp, sizeof(int)*MAX_DIM*n);
	
	VS rv = lpsolve();
	
	DB(validate(rv));
	
	DB(clears.S);
	rv.insert(rv.end(), ALL(clears));
	DB(rv.S);
	
	cout << rv.S << endl;
	REP(i, rv.S) cout << rv[i] << endl;
}