#include <algorithm>
#include <iostream>
#include <set>
#include <string>
#include <vector>
#include <map>
#include <cmath>

using namespace std;

const int MAX_RESULT_DOCUMENT_COUNT = 5;
// Новая структура 
struct QueryWords {
    set<string> plus_words;
    set<string> minus_words;
};
// Определение структуры Document
struct Document {
    int id;
    double relevance;
};
string ReadLine() {
    string s;
    getline(cin, s);
    return s;
}
int ReadLineWithNumber() {
    int result = 0;
    cin >> result;
    ReadLine();
    return result;
}
vector<string> SplitIntoWords(const string& text) {
    vector<string> words;
    string word;
    for (char c : text) {
        if (c == ' ') {
            if (!word.empty()) {
                words.push_back(word);
                word.clear();
            }
        } else {
            word += c;
        }
    }
    if (!word.empty()){
        words.push_back(word);}
    return words;
}
class SearchServer {
public:
    void SetStopWords(const string &text) {
        for (const string &word : SplitIntoWords(text))
            stop_words_.insert(word);
    }
    void AddDocument(int document_id, const string &document) {
        const vector<string> words = SplitIntoWordsNoStop(document);
        document_count_++;
        // TF
        double weight = 1.0 / words.size();
        for (const string &word : words) {
        index[word][document_id] += weight;
    }
    }
    vector<Document> FindTopDocuments(const string& raw_query) const {
        QueryWords query_words = ParseQuery(raw_query);
        auto matched_documents = FindAllDocuments(query_words.plus_words, query_words.minus_words);
        // IDF
        map<string, double> idf_map;
        for (const string &word : query_words.plus_words) {
            if (index.count(word) > 0) {
                idf_map[word] = log(static_cast<double>(document_count_) / index.at(word).size());
            }
        }
        //TF-IDF
        for (auto &doc : matched_documents) {//нельзя сюда добавить const 
            double relevance = 0.0;
            for (const string &word : query_words.plus_words) {
                if (index.count(word) > 0 && index.at(word).count(doc.id) > 0) {
                   relevance += index.at(word).at(doc.id) * idf_map.at(word);
                }
            }
           doc.relevance = relevance;
        } 
        // Сортировка по убыванию релевантности
        sort(matched_documents.begin(), matched_documents.end(),
             [](const Document &lhs, const Document &rhs) {
                 return lhs.relevance > rhs.relevance;
             });    
        if (matched_documents.size() > MAX_RESULT_DOCUMENT_COUNT)
            matched_documents.resize(MAX_RESULT_DOCUMENT_COUNT);
        return matched_documents;
    }
private:
    set<string> stop_words_;
    map<string, map<int, double>> index; // хранит TF
    int document_count_ = 0;

    bool IsStopWord(const string& word) const {
        return stop_words_.count(word) > 0;
    }
    vector<string> SplitIntoWordsNoStop(const string &text) const {
        vector<string> words;
        for (const string& word : SplitIntoWords(text)) {
            if (!IsStopWord(word))
                words.push_back(word);
        }
        return words;
    }
    QueryWords ParseQuery(const string &text) const {
        QueryWords query_words;

        for (const string& word : SplitIntoWords(text)) {
            if (word[0] == '-')
                query_words.minus_words.insert(word.substr(1));
            else
                query_words.plus_words.insert(word);
        }

        return query_words;
    }
    vector<Document> FindAllDocuments(const set<string>& plus_words,
                                      const set<string>& minus_words) const {
        map<int, double> doc_relevance;
        // Подсчитываем релевантность для всех документов, содержащих плюс-слова
        for (const string& word : plus_words) {
            if (index.find(word) != index.end()) {
                for (const auto &[doc_id, tf_value] : index.at(word)) {
                    doc_relevance[doc_id] += tf_value; //строка IDF 80
                }
            }
        }
        // Исключаем документы, содержащие минус-слова
        for (const string& word : minus_words) {
            if (index.find(word) != index.end()) {
                for (const auto &[doc_id, _] : index.at(word)) {
                    doc_relevance.erase(doc_id);
                }
            }
        }
        vector<Document> matched_documents;
        for (const auto &[doc_id, relevance] : doc_relevance) {
            matched_documents.push_back({doc_id, relevance});
        }
        return matched_documents;
    }
};
SearchServer CreateSearchServer() {
    SearchServer search_server;
    search_server.SetStopWords(ReadLine());
    const int document_count = ReadLineWithNumber();
    for (int document_id = 0; document_id < document_count; ++document_id) {
        search_server.AddDocument(document_id, ReadLine());
    }
    return search_server;
}
int main() {
    const SearchServer search_server = CreateSearchServer();
    const string query = ReadLine();
    for (const auto &[document_id, relevance] : search_server.FindTopDocuments(query)) {
        cout << "{ document_id = " << document_id << ", "
             << "relevance = " << relevance << " }" << endl;
    }
}
