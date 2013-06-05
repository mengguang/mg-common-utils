module Balancer {
	interface myBalancer {
		string sqlInsert(string sql,string table,string hashval);
		string sqlDelete(string sql,string table,string hashval);
		string sqlUpdate(string sql,string table,string hashval);
		string sqlSelect(string sql,string table,string hashval,int expire);
	};
};

