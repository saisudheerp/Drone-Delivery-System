#include <iostream>
#include <limits>
#include <queue>
#include <vector>
#include <stdexcept>
#include <string>

using namespace std;

const int MAX_WAREHOUSES = 100;
const int MAX_PRODUCTS = 100;

// -------------------- Drone classes --------------------
class Drone {
protected:
    int batterycapacity;

public:
    Drone(int batterycapacity) : batterycapacity(batterycapacity) {}
    virtual bool cancompletedelivery(int distance) const = 0;
    virtual int getdeliverycharge(int distance) const = 0;
};

class Shortdistancedrone : public Drone {
public:
    Shortdistancedrone(int batterycapacity) : Drone(batterycapacity) {}
    bool cancompletedelivery(int distance) const override { return distance <= batterycapacity; }
    int getdeliverycharge(int distance) const override { return distance * 2; }
};

class Longdistancedrone : public Drone {
public:
    Longdistancedrone(int batterycapacity) : Drone(batterycapacity) {}
    bool cancompletedelivery(int distance) const override { return distance <= batterycapacity; }
    int getdeliverycharge(int distance) const override { return distance * 3; }
};

// -------------------- Warehouse class --------------------
class Warehouse {
public:
    int id;
    Warehouse* neighbors[MAX_WAREHOUSES];
    int distances[MAX_WAREHOUSES];
    int numneighbors;
    string products[MAX_PRODUCTS];
    int numProducts;
    Shortdistancedrone shortdistancedrone;
    Longdistancedrone longdistancedrone;

    Warehouse(int id, int shortDistanceBattery, int longDistanceBattery)
        : id(id), numneighbors(0), numProducts(0),
          shortdistancedrone(shortDistanceBattery),
          longdistancedrone(longDistanceBattery) {}

    void addneighbor(Warehouse* neighbor, int distance) {
        if (numneighbors >= MAX_WAREHOUSES) throw runtime_error("Max neighbors reached");
        neighbors[numneighbors] = neighbor;
        distances[numneighbors] = distance;
        ++numneighbors;
    }

    void addproduct(const string& product) {
        if (numProducts >= MAX_PRODUCTS) throw runtime_error("Max products reached");
        products[numProducts] = product;
        ++numProducts;
    }

    bool hasProduct(const string& product) const {
        for (int i = 0; i < numProducts; ++i)
            if (products[i] == product) return true;
        return false;
    }

    bool candeliverwithdrones(int distance) const {
        return shortdistancedrone.cancompletedelivery(distance) ||
               longdistancedrone.cancompletedelivery(distance);
    }

    int calculatedeliverycharge(int distance) const {
        if (shortdistancedrone.cancompletedelivery(distance)) return shortdistancedrone.getdeliverycharge(distance);
        else if (longdistancedrone.cancompletedelivery(distance)) return longdistancedrone.getdeliverycharge(distance);
        else throw runtime_error("No drone can complete delivery.");
    }

    // Dijkstra for shortest distance
    int dijkstraCalculatedistance(Warehouse* destinationwarehouse) const {
        vector<int> minDistance(MAX_WAREHOUSES, numeric_limits<int>::max());
        minDistance[id - 1] = 0;

        priority_queue<pair<int, Warehouse*>, vector<pair<int, Warehouse*>>, greater<pair<int, Warehouse*>>> pq;
        pq.push(make_pair(0, const_cast<Warehouse*>(this)));

        while (!pq.empty()) {
            int currentDistance = pq.top().first;
            Warehouse* currentWarehouse = pq.top().second;
            pq.pop();

            if (currentWarehouse == destinationwarehouse) return currentDistance;

            for (int i = 0; i < currentWarehouse->numneighbors; ++i) {
                Warehouse* neighbor = currentWarehouse->neighbors[i];
                int distanceToNeighbor = currentWarehouse->distances[i];
                int newDistance = currentDistance + distanceToNeighbor;

                if (newDistance < minDistance[neighbor->id - 1]) {
                    minDistance[neighbor->id - 1] = newDistance;
                    pq.push(make_pair(newDistance, neighbor));
                }
            }
        }
        return numeric_limits<int>::max();
    }
};

// -------------------- DeliveryOrder class --------------------
class DeliveryOrder {
public:
    Warehouse* sourcewarehouse;
    Warehouse* destinationwarehouse;
    string product;
    bool productAvailable;
    int distance;
    int deliveryCharge;

    DeliveryOrder(Warehouse* sourcewarehouse, Warehouse* destinationwarehouse, const string& product, int distance)
        : sourcewarehouse(sourcewarehouse), destinationwarehouse(destinationwarehouse),
          product(product), productAvailable(false), distance(distance), deliveryCharge(0) {}

    void checkProductAvailability() {
        productAvailable = sourcewarehouse->hasProduct(product);
    }

    void calculatedeliverycharge() {
        try {
            deliveryCharge = destinationwarehouse->calculatedeliverycharge(distance);
        } catch (const runtime_error& ex) {
            cout << "Error calculating delivery charge: " << ex.what() << endl;
        }
    }

    void printDeliveryDetails() {
        cout << "Delivery Details:" << endl;
        cout << "Source Warehouse: " << sourcewarehouse->id << endl;
        cout << "Destination Warehouse: " << destinationwarehouse->id << endl;
        cout << "Product: " << product << endl;
        cout << "Product Available: " << (productAvailable ? "Yes" : "No") << endl;
        cout << "Distance: " << distance << " units" << endl;
        cout << "Delivery Charge: $" << deliveryCharge << endl;
    }
};

// -------------------- DroneDeliverySystem class --------------------
class Dronedeliverysystem {
private:
    Warehouse* warehouses[MAX_WAREHOUSES];
    int numWarehouses;
    DeliveryOrder* deliveryOrders[MAX_WAREHOUSES];
    int numdeliveryorders;

    // Optimized: pick nearest warehouse and store distance
    pair<Warehouse*, int> findNearestWarehouseWithProduct(const string& product, Warehouse* destination) {
        Warehouse* nearest = nullptr;
        int minDist = numeric_limits<int>::max();
        for (int i = 0; i < numWarehouses; ++i) {
            if (warehouses[i]->hasProduct(product)) {
                int dist = warehouses[i]->dijkstraCalculatedistance(destination);
                if (dist < minDist) {
                    minDist = dist;
                    nearest = warehouses[i];
                }
            }
        }
        return make_pair(nearest, minDist);
    }

public:
    Dronedeliverysystem() : numWarehouses(0), numdeliveryorders(0) {}

    Warehouse* createwarehouse(int id, int shortDistanceBattery, int longDistanceBattery) {
        if (numWarehouses >= MAX_WAREHOUSES) throw runtime_error("Max warehouses reached");
        Warehouse* warehouse = new Warehouse(id, shortDistanceBattery, longDistanceBattery);
        warehouses[numWarehouses++] = warehouse;
        return warehouse;
    }

    void addwarehouseneighbor(Warehouse* warehouse, Warehouse* neighbor, int distance) {
        warehouse->addneighbor(neighbor, distance);
    }

    void addwarehouseproduct(Warehouse* warehouse, const string& product) {
        warehouse->addproduct(product);
    }

    void createdeliveryorder(Warehouse* destinationwarehouse, const string& product) {
        if (numdeliveryorders >= MAX_WAREHOUSES) throw runtime_error("Max delivery orders reached");

        pair<Warehouse*, int> result = findNearestWarehouseWithProduct(product, destinationwarehouse);
        Warehouse* sourcewarehouse = result.first;
        int distance = result.second;

        if (sourcewarehouse != nullptr) {
            DeliveryOrder* deliveryOrder = new DeliveryOrder(sourcewarehouse, destinationwarehouse, product, distance);
            deliveryOrder->checkProductAvailability();
            deliveryOrder->calculatedeliverycharge();
            deliveryOrders[numdeliveryorders++] = deliveryOrder;
        } else {
            cout << "Product '" << product << "' not available in any warehouse. Skipping delivery." << endl;
        }
    }

    void processdeliveries() {
        for (int i = 0; i < numdeliveryorders; ++i) {
            deliveryOrders[i]->printDeliveryDetails();
            cout << endl;
        }
    }

    ~Dronedeliverysystem() {
        for (int i = 0; i < numWarehouses; ++i) delete warehouses[i];
        for (int i = 0; i < numdeliveryorders; ++i) delete deliveryOrders[i];
    }
};

// -------------------- Main function --------------------
int main() {
    try {
        Dronedeliverysystem deliverySystem;

        Warehouse* warehouse1 = deliverySystem.createwarehouse(1, 5, 10);
        Warehouse* warehouse2 = deliverySystem.createwarehouse(2, 8, 15);
        Warehouse* warehouse3 = deliverySystem.createwarehouse(3, 12, 20);
        Warehouse* warehouse4 = deliverySystem.createwarehouse(4, 6, 10);

        deliverySystem.addwarehouseneighbor(warehouse1, warehouse2, 5);
        deliverySystem.addwarehouseneighbor(warehouse1, warehouse3, 10);
        deliverySystem.addwarehouseneighbor(warehouse2, warehouse3, 3);
        deliverySystem.addwarehouseneighbor(warehouse2, warehouse4, 2);
        deliverySystem.addwarehouseneighbor(warehouse3, warehouse4, 6);

        deliverySystem.addwarehouseproduct(warehouse1, "Product A");
        deliverySystem.addwarehouseproduct(warehouse1, "Product B");
        deliverySystem.addwarehouseproduct(warehouse2, "Product B");
        deliverySystem.addwarehouseproduct(warehouse3, "Product C");
        deliverySystem.addwarehouseproduct(warehouse4, "Product A");
        deliverySystem.addwarehouseproduct(warehouse4, "Product C");

        deliverySystem.createdeliveryorder(warehouse2, "Product A");
        deliverySystem.createdeliveryorder(warehouse3, "Product B");
        deliverySystem.createdeliveryorder(warehouse4, "Product C");

        deliverySystem.processdeliveries();
    }
    catch (const exception& ex) {
        cout << "An error occurred: " << ex.what() << endl;
    }

    return 0;
}
