/*
サンプル初期実装
移動先に資源が無い回収車を、ランダムに選んだ出現中の資源へと移動させる
ただしこのとき２台以上の回収車が同じ資源を選ばないようにする
*/
#include <stdlib.h>
#include <iostream>
#include <string>
#include <vector>
#include <set>
#include <thread>
#include <chrono>
#include <queue>
#include <random>
#include <algorithm>
using namespace std;

mt19937 mt;

struct AgentMove {
	double x, y;
	int t;
};

struct Agent {
	vector<AgentMove> move;
};

struct Resource {
	int id, x, y, t0, t1;
	string type;
	int weight;
};

struct ResourceWithAmount : public Resource {
	double amount;
};

struct OwnedResource {
	string type;
	double amount;
};

struct Game {
	int now;
	vector<Agent> agent;
	vector<Resource> resource;
	int next_resource;
	vector<OwnedResource> owned_resource;
};

struct Move {
	int now;
	vector<AgentMove> move;
};

struct Resources {
	vector<ResourceWithAmount> resource;
};

Game call_game() {
	cout << "game" << endl;
	Game res;
	int num_agent, num_resource, num_owned_resource;
	cin >> res.now >> num_agent >> num_resource >> res.next_resource >> num_owned_resource;
	res.agent.resize(num_agent);
	for (auto& a : res.agent) {
		int num_move;
		cin >> num_move;
		a.move.resize(num_move);
		for (auto& m : a.move) {
			cin >> m.x >> m.y >> m.t;
		}
	}
	res.resource.resize(num_resource);
	for (auto& r : res.resource) {
		cin >> r.id >> r.x >> r.y >> r.t0 >> r.t1 >> r.type >> r.weight;
	}
	res.owned_resource.resize(num_owned_resource);
	for (auto& o : res.owned_resource) {
		cin >> o.type >> o.amount;
	}
	return res;
}

Move read_move() {
	Move res;
	int num_move;
	cin >> res.now >> num_move;
	res.move.resize(num_move);
	for (auto& m : res.move) {
		cin >> m.x >> m.y >> m.t;
	}
	return res;
}

Move call_move(int index, int x, int y) {
	cout << "move " << index << " " << x << " " << y << endl;
	return read_move();
}

Move call_will_move(int index, int x, int y, int t) {
	cout << "will_move " << index << " " << x << " " << y << " " << t << endl;
	return read_move();
}

Resources call_resources(vector<int> ids) {
	cout << "resources";
	for (auto id : ids) {
		cout << " " << id;
	}
	cout << endl;
	Resources res;
	int num_resource;
	cin >> num_resource;
	res.resource.resize(num_resource);
	for (auto& r : res.resource) {
		cin >> r.id >> r.x >> r.y >> r.t0 >> r.t1 >> r.type >> r.weight >> r.amount;
	}
	return res;
}

double calc_score(const Game& game) {
	vector<double> a;
	for (const auto& o : game.owned_resource) {
		a.push_back(o.amount);
	}
	sort(a.begin(), a.end());
	return a[0] + 0.1 * a[1] + 0.01 * a[2];
}

template<typename T, bool reverse = false> inline auto Argsort(const vector<T>& vec) {
	vector<int> res(vec.size());
	iota(res.begin(), res.end(), 0);
	sort(res.begin(), res.end(), [&](const int& l, const int& r) {
		return reverse ? vec[l] > vec[r] : vec[l] < vec[r];
	});
	return res;
}

auto agent_positions = array<pair<int, int>, 5>();
auto agent_end_times = array<int, 5>();

struct Bot {
	Game game;
	void solve() {
		for (;;) {
			game = call_game();

			// 情報出力
			for (const auto& o : game.owned_resource) {
				fprintf(stderr, "%s: %.2f ", o.type.c_str(), o.amount);
			}
			fprintf(stderr, "Score: %.2f\n", calc_score(game));

			// 資源の場所
			set<pair<int,int>> resource_positions;
			for (const auto& r : game.resource) {
				if (r.t0 <= game.now && game.now < r.t1) {
					resource_positions.insert({r.x, r.y});
				}
			}

			vector<int> index_list;  // 1.0 秒以内にそこに資源が無くなるようなエージェントのリスト
			for(int i=0; i<5; i++){
				const auto& m = game.agent[i].move.back();
				m.t + 
			}



			for (int i = 0; i < 5; ++ i) {
				const auto& m = game.agent[i].move.back();
				if (resource_positions.count({m.x, m.y})) {
					resource_positions.erase({m.x, m.y});
				} else {
					index_list.push_back(i+1);
				}
			}
			auto f = [](double x1, double y1, double x2, double y2){
				return (int)(100.0 * sqrt((x1-x2)*(x1-x2) + (y1-y2)*(y1-y2)));
			};
			// 動かす
			for (int index : index_list) {
				const auto& agent = game.agent[index-1];
				if (resource_positions.empty()) break;
				
				auto importances = vector<double>();
				for (const auto& r : game.resource) {
					const auto coef_type = r.type == "A" ? 1.0 : r.type == "B" ? 2.5 : 3.0;
					const auto ta = game.now + f(r.x, r.y, agent.move.back().x, agent.move.back().y);  // 到着時刻
					if(ta >= r.t1) {
						importances.push_back(0.0);
						continue;
					}
					const auto tb = max(r.t0, ta);
					const auto tws = r.t1 - tb;
					const auto twh = r.t1 - game.now;
					const auto importance = (double)tws / (double)twh * coef_type * r.weight;
					importances.push_back(importance);
				}
				const auto important_order = Argsort<double, true>(importances);


				//int r = uniform_int_distribution<>(0, resource_positions.size()-1)(mt);
				int r = important_order[0];
				const auto x = game.resource[r].x;
				const auto y = game.resource[r].y;
				call_move(index, x, y);
				agent_positions[index-1] = make_pair(x, y);
				agent_end_times[index-1] = game.resource[r].t1;
			}

			this_thread::sleep_for(chrono::milliseconds(1000));
		}
	}
};

int main() {
	random_device seed_gen;
	mt = mt19937(seed_gen());

	Bot bot;
	bot.solve();
}