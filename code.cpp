#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <ctime>
#include <sstream>
#include <map>
#include <algorithm>
#include <cstdio>
#include <iomanip>

using namespace std;

// ==================== DATE UTILITIES ====================
string todayDate() {
    time_t t = time(nullptr);
    tm* now = localtime(&t);
    char buf[11];
    strftime(buf, sizeof(buf), "%Y-%m-%d", now);
    return string(buf);
}

int daysUntil(const string& deadline) {
    int y, m, d;
    if (sscanf(deadline.c_str(), "%d-%d-%d", &y, &m, &d) != 3) return 999;
    struct tm dl_tm = {};
    dl_tm.tm_year = y - 1900;
    dl_tm.tm_mon = m - 1;
    dl_tm.tm_mday = d;
    dl_tm.tm_isdst = -1;
    time_t dl = mktime(&dl_tm);

    time_t now = time(nullptr);
    tm* today_tm = localtime(&now);
    today_tm->tm_hour = 0;
    today_tm->tm_min = 0;
    today_tm->tm_sec = 0;
    today_tm->tm_isdst = -1;
    time_t today = mktime(today_tm);

    return (int)(difftime(dl, today) / (60.0 * 60.0 * 24.0));
}

// ==================== MISSION HIERARCHY ====================
class Mission {
protected:
    string taskName;
    string deadline;
    string category;
    int avoidanceCount;

public:
    Mission() : taskName(""), deadline(""), category(""), avoidanceCount(0) {}
    Mission(string name, string dl, string cat)
        : taskName(name), deadline(dl), category(cat), avoidanceCount(0) {}
    virtual ~Mission() {}

    string getTaskName() const { return taskName; }
    string getDeadline() const { return deadline; }
    string getCategory() const { return category; }
    int getAvoidanceCount() const { return avoidanceCount; }
    void incrementAvoidance() { avoidanceCount++; }

    virtual string getType() const = 0;

    virtual void display() const {
        cout << "  [" << getType() << "] " << taskName
             << " | Deadline: " << deadline
             << " | Avoided: " << avoidanceCount << "x\n";
    }

    string toFile() const {
        return taskName + "|" + deadline + "|" + category + "|" + to_string(avoidanceCount);
    }
};

class StudyMission : public Mission {
public:
    StudyMission() : Mission() {}
    StudyMission(string n, string d) : Mission(n, d, "Study") {}
    string getType() const override { return "STUDY"; }
};

class AssignmentMission : public Mission {
public:
    AssignmentMission() : Mission() {}
    AssignmentMission(string n, string d) : Mission(n, d, "Assignment") {}
    string getType() const override { return "ASSIGNMENT"; }
};

class ProjectMission : public Mission {
public:
    ProjectMission() : Mission() {}
    ProjectMission(string n, string d) : Mission(n, d, "Project") {}
    string getType() const override { return "PROJECT"; }
};

Mission* createMission(const string& line) {
    stringstream ss(line);
    string name, dl, cat, count;
    getline(ss, name, '|');
    getline(ss, dl, '|');
    getline(ss, cat, '|');
    getline(ss, count, '|');

    Mission* m;
    if (cat == "Study") m = new StudyMission(name, dl);
    else if (cat == "Assignment") m = new AssignmentMission(name, dl);
    else m = new ProjectMission(name, dl);

    int c = stoi(count);
    for (int i = 0; i < c; i++) m->incrementAvoidance();
    return m;
}

// ==================== AVOIDANCE ENTRY ====================
class AvoidanceEntry {
    string taskName;
    string originalExcuse;
    string realReason;
    string distraction;
    int durationAvoided;
    string date;
    bool contradiction;

public:
    AvoidanceEntry() : durationAvoided(0), contradiction(false) {}
    AvoidanceEntry(string task, string orig, string real, string dist, int dur, string dt, bool contra)
        : taskName(task), originalExcuse(orig), realReason(real),
          distraction(dist), durationAvoided(dur), date(dt), contradiction(contra) {}

    string getTaskName() const { return taskName; }
    string getOriginalExcuse() const { return originalExcuse; }
    string getRealReason() const { return realReason; }
    string getDistraction() const { return distraction; }
    int getDuration() const { return durationAvoided; }
    string getDate() const { return date; }
    bool hasContradiction() const { return contradiction; }

    string toFile() const {
        return taskName + "|" + originalExcuse + "|" + realReason + "|" +
               distraction + "|" + to_string(durationAvoided) + "|" + date + "|" +
               (contradiction ? "1" : "0");
    }

    static AvoidanceEntry fromFile(const string& line) {
        stringstream ss(line);
        string task, orig, real, dist, dur, dt, contra;
        getline(ss, task, '|');
        getline(ss, orig, '|');
        getline(ss, real, '|');
        getline(ss, dist, '|');
        getline(ss, dur, '|');
        getline(ss, dt, '|');
        getline(ss, contra, '|');
        return AvoidanceEntry(task, orig, real, dist, stoi(dur), dt, contra == "1");
    }
};

// ==================== INTERROGATOR (USP 1) ====================
class Interrogator {
public:
    string askRealReason() {
        cout << "\n==========================================\n";
        cout << "       INTERROGATION ROOM\n";
        cout << "==========================================\n";
        cout << "SPY: You logged an avoidance. Let's probe deeper.\n";
        cout << "SPY: What is the REAL reason?\n";
        cout << "  1. Task feels too big\n";
        cout << "  2. Fear of failing\n";
        cout << "  3. Genuinely tired\n";
        cout << "  4. Actually distracted\n";
        cout << "Your answer (1-4): ";
        int ans;
        cin >> ans;
        cin.ignore();
        switch (ans) {
            case 1: return "Task too big";
            case 2: return "Fear of failing";
            case 3: return "Genuinely tired";
            case 4: return "Distracted";
            default: return "Unknown";
        }
    }

    bool checkContradiction(const string& original, const string& real) {
        if (original == "Too tired") return real != "Genuinely tired";
        if (original == "Too hard")  return real != "Task too big" && real != "Fear of failing";
        if (original == "Distracted") return real != "Distracted";
        return false;
    }

    void reportContradiction(const string& task, const string& original, const string& real) {
        cout << "\n  !! CONTRADICTION DETECTED !!\n";
        cout << "  You said: \"" << original << "\"\n";
        cout << "  Real reason: \"" << real << "\"\n";
        cout << "  SPY NOTE: Your excuse doesn't match your truth, agent.\n";
        cout << "  Flagging this in your dossier.\n";
    }
};

// ==================== MOLE DETECTOR (USP 2) ====================
class MoleDetector {
public:
    pair<string, int> findMole(const vector<AvoidanceEntry>& logs) {
        map<string, int> freq;
        for (const auto& e : logs) freq[e.getDistraction()]++;
        string mole = "None";
        int maxCount = 0;
        for (const auto& p : freq) {
            if (p.second > maxCount) { maxCount = p.second; mole = p.first; }
        }
        return {mole, maxCount};
    }

    void displayMoleHunt(const vector<AvoidanceEntry>& logs) {
        cout << "\n==========================================\n";
        cout << "           MOLE HUNT\n";
        cout << "==========================================\n";
        if (logs.empty()) { cout << "  No data yet. Log some avoidances first.\n"; return; }

        auto mole = findMole(logs);
        if (mole.second == 0) { cout << "  No mole identified yet.\n"; return; }

        cout << "  MOLE IDENTIFIED: \"" << mole.first << "\"\n";
        cout << "  APPEARANCES: " << mole.second << " times\n";
        cout << "  MISSION: Next 7 days, log every mole appearance.\n";
        cout << "  TARGET: Keep sightings below " << (mole.second / 2) << ".\n";

        if (mole.second >= 8) cout << "  REWARD: FIELD AGENT rank\n";
        else if (mole.second >= 4) cout << "  REWARD: JUNIOR AGENT rank\n";
        else cout << "  REWARD: TRAINEE completion\n";

        cout << "  STATUS: ACTIVE\n";
    }
};

// ==================== THREAT ANALYZER (USP 3) ====================
class ThreatAnalyzer {
public:
    string getThreatLevel(const Mission& m) {
        int days = daysUntil(m.getDeadline());
        int avoid = m.getAvoidanceCount();

        if (days <= 0) return "CRITICAL";
        if (days <= 2) return "CRITICAL";
        if (days <= 5) return avoid >= 2 ? "CRITICAL" : "ELEVATED";
        if (days <= 10) return avoid >= 3 ? "ELEVATED" : "GUARDED";
        return avoid >= 3 ? "GUARDED" : "LOW";
    }

    void displayThreatBoard(const vector<Mission*>& missions) {
        cout << "\n==========================================\n";
        cout << "           THREAT BOARD\n";
        cout << "==========================================\n";
        if (missions.empty()) { cout << "  No active missions. Add one first.\n"; return; }

        for (const auto& m : missions) {
            string level = getThreatLevel(*m);
            cout << "  " << m->getTaskName()
                 << " — THREAT: " << level;
            if (level == "CRITICAL") cout << "  [!!!]";
            else if (level == "ELEVATED") cout << "  [!]";
            cout << "\n";
            cout << "     Deadline: " << m->getDeadline()
                 << " | Avoided: " << m->getAvoidanceCount() << "x\n";
        }
    }
};

// ==================== DOSSIER ====================
class Dossier {
public:
    void generateReport(const vector<Mission*>& missions, const vector<AvoidanceEntry>& logs) {
        cout << "\n==========================================\n";
        cout << "       AGENT DOSSIER\n";
        cout << "==========================================\n";

        if (logs.empty()) { cout << "  No data yet. Log some avoidances first.\n"; return; }

        cout << "\n  MISSIONS AVOIDED: " << logs.size() << " this month\n";

        // Top excuse
        map<string, int> excuseFreq;
        for (const auto& e : logs) excuseFreq[e.getOriginalExcuse()]++;
        string topExcuse; int topCount = 0;
        for (const auto& p : excuseFreq)
            if (p.second > topCount) { topCount = p.second; topExcuse = p.first; }

        cout << "  TOP EXCUSE: \"" << topExcuse << "\" (" << topCount << "x)\n";

        // Contradictions
        int contradictions = 0;
        for (const auto& e : logs) if (e.hasContradiction()) contradictions++;
        cout << "  CONTRADICTIONS DETECTED: " << contradictions << "\n";

        // Contradiction pattern
        if (contradictions > 0) {
            map<string, int> liePatterns;
            for (const auto& e : logs) {
                if (e.hasContradiction())
                    liePatterns[e.getOriginalExcuse() + " -> " + e.getRealReason()]++;
            }
            string topLie; int topLieCount = 0;
            for (const auto& p : liePatterns)
                if (p.second > topLieCount) { topLieCount = p.second; topLie = p.first; }
            cout << "  LIE PATTERN: \"" << topLie << "\" (" << topLieCount << "x)\n";
        }

        // Mole
        MoleDetector md;
        auto mole = md.findMole(logs);
        if (mole.second > 0)
            cout << "  MOLE: \"" << mole.first << "\" (" << mole.second << " sightings)\n";

        // Threat
        ThreatAnalyzer ta;
        int critical = 0, elevated = 0;
        for (const auto& m : missions) {
            string lvl = ta.getThreatLevel(*m);
            if (lvl == "CRITICAL") critical++;
            else if (lvl == "ELEVATED") elevated++;
        }
        cout << "  THREAT LEVEL: " << critical << " CRITICAL, " << elevated << " ELEVATED\n";

        // Verdict
        cout << "\n  VERDICT:\n";
        if (contradictions > 0 && mole.second >= 5) {
            cout << "  You're not lazy. You're avoiding tasks that feel large,\n";
            cout << "  and using \"" << mole.first << "\" as your escape hatch.\n";
        } else if (contradictions > 0) {
            cout << "  Your excuses are inconsistent with your real reasons.\n";
            cout << "  That's not procrastination — that's self-deception.\n";
        } else if (mole.second >= 5) {
            cout << "  You're not lazy. \"" << mole.first << "\" is your mole.\n";
            cout << "  Cut it and you'll see the difference.\n";
        } else {
            cout << "  Agent, you're performing well. Stay sharp.\n";
        }
    }
};

// ==================== SPY ENGINE ====================
class SpyEngine {
    vector<Mission*> missions;
    vector<AvoidanceEntry> logs;
    Interrogator interrogator;
    MoleDetector moleDetector;
    ThreatAnalyzer threatAnalyzer;
    Dossier dossier;

public:
    ~SpyEngine() { for (auto m : missions) delete m; }

    void addMission() {
        cout << "\n--- NEW MISSION ---\n";
        cout << "Mission type:\n  1. Study\n  2. Assignment\n  3. Project\nChoice: ";
        int ch; cin >> ch; cin.ignore();
        cout << "Task name: ";
        string name; getline(cin, name);
        cout << "Deadline (YYYY-MM-DD): ";
        string dl; getline(cin, dl);

        Mission* m;
        if (ch == 1) m = new StudyMission(name, dl);
        else if (ch == 2) m = new AssignmentMission(name, dl);
        else m = new ProjectMission(name, dl);

        missions.push_back(m);
        cout << "Mission \"" << name << "\" added to your watchlist.\n";
    }

    void logAvoidance() {
        if (missions.empty()) { cout << "No missions. Add one first.\n"; return; }

        cout << "\n--- LOG AVOIDANCE ---\n";
        cout << "Which mission did you avoid?\n";
        for (size_t i = 0; i < missions.size(); i++)
            cout << "  " << i + 1 << ". " << missions[i]->getTaskName() << "\n";
        cout << "Choice: ";
        int ch; cin >> ch; cin.ignore();
        if (ch < 1 || ch > (int)missions.size()) { cout << "Invalid.\n"; return; }

        Mission* m = missions[ch - 1];

        cout << "Duration avoided (hours): ";
        int dur; cin >> dur; cin.ignore();

        cout << "\nWhat was your original excuse?\n";
        cout << "  1. Too tired\n  2. Too hard\n  3. Distracted\n";
        cout << "Choice: ";
        int exc; cin >> exc; cin.ignore();
        string original;
        if (exc == 1) original = "Too tired";
        else if (exc == 2) original = "Too hard";
        else original = "Distracted";

        string real = interrogator.askRealReason();
        bool contra = interrogator.checkContradiction(original, real);
        if (contra) interrogator.reportContradiction(m->getTaskName(), original, real);

        cout << "\nWhat were you ACTUALLY doing? ";
        string distraction; getline(cin, distraction);

        logs.push_back(AvoidanceEntry(m->getTaskName(), original, real,
                                      distraction, dur, todayDate(), contra));
        m->incrementAvoidance();

        cout << "\nEntry logged. Your dossier has been updated.\n";
    }

    void showThreatBoard() { threatAnalyzer.displayThreatBoard(missions); }
    void showMoleHunt() { moleDetector.displayMoleHunt(logs); }
    void showDossier() { dossier.generateReport(missions, logs); }

    void save() {
        ofstream mf("missions.txt");
        for (auto m : missions) mf << m->toFile() << "\n";
        mf.close();

        ofstream lf("logs.txt");
        for (auto& e : logs) lf << e.toFile() << "\n";
        lf.close();
    }

    void load() {
        missions.clear(); logs.clear();
        ifstream mf("missions.txt");
        string line;
        while (getline(mf, line)) if (!line.empty()) missions.push_back(createMission(line));
        mf.close();

        ifstream lf("logs.txt");
        while (getline(lf, line)) if (!line.empty()) logs.push_back(AvoidanceEntry::fromFile(line));
        lf.close();
    }
};

// ==================== MAIN ====================
int main() {
    SpyEngine engine;
    engine.load();

    cout << "==========================================\n";
    cout << "   PROCRASTINATION SPY: Operation Deadline\n";
    cout << "==========================================\n";
    cout << "Welcome, Agent.\n";

    int choice;
    while (true) {
        cout << "\n--- MAIN MENU ---\n";
        cout << "1. Add New Mission\n";
        cout << "2. Log Avoidance (Interrogation Room)\n";
        cout << "3. View Threat Board\n";
        cout << "4. View Mole Hunt\n";
        cout << "5. View Agent Dossier\n";
        cout << "6. Save & Exit\n";
        cout << "Choice: ";
        cin >> choice;
        cin.ignore();

        switch (choice) {
            case 1: engine.addMission(); break;
            case 2: engine.logAvoidance(); break;
            case 3: engine.showThreatBoard(); break;
            case 4: engine.showMoleHunt(); break;
            case 5: engine.showDossier(); break;
            case 6:
                engine.save();
                cout << "\nDossier saved. Mission complete, Agent.\n";
                return 0;
            default: cout << "Invalid choice.\n";
        }
    }
}