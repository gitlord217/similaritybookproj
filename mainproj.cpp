#include <iostream>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <vector>
#include <algorithm>
#include <set>
#include <cmath>
#include <string>
#include <iomanip>
#include <filesystem>

using namespace std;
namespace fs = std::filesystem;

const string directoryPath = "./Book-Txt/"; // Path to the directory which contains all our txt files
const set<string> commonWords = {"A", "AND", "AN", "OF", "IN", "THE"}; // Common words to exclude

// Function to calculate the top 100 normalized word frequencies
unordered_map<string, double> calcTopHundred(const string& fname) {
    unordered_map<string, int> wordCount;
    int totalWords = 0;
    ifstream file(fname);
    string word;

    if (!file.is_open()) {
        cerr << "Error opening file: " << fname << endl;
        return {};
    }

    // Process words
    while (file >> word) {
        for (char& c : word) {
            c = toupper(c); // Convert each character to uppercase
        }

        if (!commonWords.count(word)) { // Exclude common words
            wordCount[word]++;
            totalWords++;
        }
    }

    // Normalize frequencies and keep only top 100
    vector<pair<string, double>> freq;
    for (const auto& x: wordCount) {
        double normalizedfrq = (double)x.second / totalWords; // Calculate normalized frequency
        freq.push_back(pair<string, double>(x.first, normalizedfrq)); // Add to vector
    }

    // Sort the freq vector
    sort(freq.begin(), freq.end(), [](const auto& a, const auto& b) {
        return a.second > b.second;
    });

    // Creating an unordered_map for top 100 words
    unordered_map<string, double> topFreqwords;
    size_t count = 0; // Counter for top words
    for (size_t i = 0; i < freq.size() && count < 100; i++) {
        topFreqwords[freq[i].first] = freq[i].second; // Add to top 100 map
        count++; // Increment the counter
    }

    return topFreqwords; // Return the top 100 words
}

// Function to calculate similarity index between two books
double calculateSimilarity(const unordered_map<string, double>& book1, const unordered_map<string, double>& book2) {
    double similarity = 0.0;
    for (const auto& word : book1) {
        if (book2.count(word.first) > 0) {
            similarity += min(word.second, book2.at(word.first));
        }
    }
    return similarity;
}

// Function to write the top 100 words for each book and the similarity matrix to output files
void writeOutputFiles(const vector<unordered_map<string, double>>& books) {
    ofstream freqFile("frequencies.txt");
    ofstream simFile("similarity_matrix.txt");

    if (!freqFile || !simFile) {
        cerr << "Error creating output files." << endl;
        return;
    }

    // Write top 100 words for each book
    for (size_t i = 0; i < books.size(); i++) {
        freqFile << "Book " << i + 1 << ":\n";
        for (const auto& word : books[i]) {
            freqFile << word.first << ": " << fixed << setprecision(4) << word.second << endl;
        }
        freqFile << endl;
    }

    // Calculate and write similarity matrix
    vector<pair<double, pair<int, int>>> similarityPairs;
    for (size_t i = 0; i < books.size(); i++) {
        for (size_t j = i + 1; j < books.size(); j++) {
            double similarity = calculateSimilarity(books[i], books[j]);
            simFile << similarity << "\t";
            similarityPairs.push_back(pair<double, pair<int, int>>(similarity, pair<int, int>(i + 1, j + 1)));
        }
        simFile << endl;
    }

    // Sort and output top 10 most similar pairs
    sort(similarityPairs.begin(), similarityPairs.end(), greater<>());
    cout << "Top 10 Most Similar Book Pairs:\n";
    for (int i = 0; i < 10 && i < similarityPairs.size(); i++) {
        cout << "Book " << similarityPairs[i].second.first << " and Book " 
             << similarityPairs[i].second.second << " - have similarity: " 
             << fixed << setprecision(4) << similarityPairs[i].first << endl;
    }
}

int main() {
    vector<unordered_map<string, double>> books;

    // First work on each book's top 100 normalized word frequencies
    for (const auto& x : fs::directory_iterator(directoryPath)) {
        if (x.is_regular_file() && x.path().extension() == ".txt") {
            string filename = (string)x.path();
            cout << "Processing " << filename << "...\n";
            books.push_back(calcTopHundred(filename));
        }
    }

    // Write results to output files
    writeOutputFiles(books);

    return 0;
}
