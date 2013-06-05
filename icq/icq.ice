module MICQ {
    interface icq {
        bool create(string name);
        string insert(string name,string data);
        string get(string name,string pos);
        string getNext(string name,string pos);
        string getLatest(string name);
        string getOldest(string name);
        string truncate(string name);
    };
};
