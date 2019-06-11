#include<iostream>
#include<ctime>
#include<vector>
#include<math.h>
#include<list>
#include<iterator>
#include<fstream>
#include<sstream>

using namespace std;

const int numTrafficLights = 3;
const int maxCarsPassed = 20;
const int hoursGenerated = 3;
const int measurementsPerHour = 12;
const string fileName = "testdata.csv";

class TrafficEntry {
    public:
        time_t timestamp;
        int numCarsPassed;
        int trafficLightId;

        TrafficEntry(time_t timestamp, int numCarsPassed, int trafficLightId) {
            this->timestamp = timestamp;
            this->numCarsPassed = numCarsPassed;
            this->trafficLightId = trafficLightId;
        }
        string ToString() {
            stringstream stringStream;
            stringStream << this->timestamp << "," << numCarsPassed << "," << trafficLightId << "\n";
            return stringStream.str();
        }
};

class TrafficLight {
    public:
        int id;
        int carsPassed;
        static int nextTrafficId;
        TrafficLight() : TrafficLight(TrafficLight::nextTrafficId) {}
        TrafficLight(int id) : TrafficLight(id, 0) {}
        TrafficLight(int id, int carsPassed) {
            this->carsPassed = carsPassed;
            this->id = id;
            if (this->id >= TrafficLight::nextTrafficId) {
                TrafficLight::nextTrafficId = this->id + 1;
            }
        }

        TrafficEntry Simulate(time_t timestamp) {
            int carsPassed = rand() % maxCarsPassed + 1;
            return TrafficEntry {timestamp, carsPassed, this->id};
        }
};

class TrafficProducer {
    TrafficLight trafficLights[numTrafficLights];

    public:
        void Simulate() {
            time_t currentTime = time(NULL);
            int timeInterval = 60 / measurementsPerHour;
            ofstream dataFile;
            dataFile.open(fileName);

            for (int i = 0; i < measurementsPerHour * hoursGenerated; i++) {
                for (int j = 0; j < numTrafficLights; j++) {
                    TrafficEntry entry = this->trafficLights[j].Simulate(currentTime);
                    dataFile << entry.ToString();
                }
                currentTime += timeInterval * 60;
            }

            dataFile.close();
        }
};

class TrafficConsumer {
    list<TrafficLight> trafficLights;

    public:
        void ProcessSequentially() {
            ifstream dataFile;
            dataFile.open(fileName);
            string line;

            while (dataFile >> line) {
                stringstream lineStream(line);
                string item[3];
                stringstream itemStream;
                int i = 0;
                while (getline(lineStream, item[i], ',')) {
                    item[i] = item[i].c_str();
                    i++;
                }

                TrafficEntry entry {(time_t) stoi(item[0]), stoi(item[1]), stoi(item[2])};

                // Find traffic light
                list<TrafficLight>::iterator light;
                bool foundLight = false;
                for (light = trafficLights.begin(); light != trafficLights.end(); ++light) {
                    if (light->id == entry.trafficLightId) {
                        light->carsPassed += entry.numCarsPassed;
                        foundLight = true;
                        break;
                    }
                }

                if (!foundLight) {
                    trafficLights.push_back(TrafficLight {entry.trafficLightId, entry.numCarsPassed});
                }
            }

            list<TrafficLight>::iterator light;

            // Sorting TrafficLights by cars passed
            trafficLights.sort([](const TrafficLight & a, const TrafficLight & b) {
                return a.carsPassed > b.carsPassed;
            });

            cout << "Cars Passed\n---------------\n";
            for (light = trafficLights.begin(); light != trafficLights.end(); ++light) {
                cout << "TrafficLight " << light->id << ": " << light->carsPassed << "\n";
            }

            dataFile.close();
        }
};

int TrafficLight::nextTrafficId = 1;

int main() {
    TrafficProducer producer;
    producer.Simulate();

    TrafficConsumer consumer;
    consumer.ProcessSequentially();

    return 0;
}
