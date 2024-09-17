#include <iostream>
#include <limits>
#include <queue>

using namespace std;

const int MAX_WAREHOUSES = 100;
const int MAX_PRODUCTS = 100;


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

    bool cancompletedelivery(int distance) const override {
        return distance <= batterycapacity;
    }

    int getdeliverycharge(int distance) const override {
        return distance * 2;  
    }
};

// Long Distance Drone derived from base Drone class
class Longdistancedrone : public Drone {
public:
    Longdistancedrone(int batterycapacity) : Drone(batterycapacity) {}

    bool cancompletedelivery(int distance) const override {
        return distance <= batterycapacity;
    }

    int getdeliverycharge(int distance) const override {
        return distance * 3;  
    }
};


class Warehouse {
public:
    int id;
    Warehouse* neighbors[MAX_WAREHOUSES];
    int distances[MAX_WAREHOUSES];
    int numneighbors;
    string products[MAX_PRODUCTS];
    int numProducts;
    Shortdistancedrone Shortdistancedrone;
    Longdistancedrone Longdistancedrone;

    Warehouse(int id, int shortDistanceBattery, int longDistanceBattery)
        : id(id), numneighbors(0), numProducts(0),
          Shortdistancedrone(shortDistanceBattery),
          Longdistancedrone(longDistanceBattery) {}

    void addneighbor(Warehouse* neighbor, int distance) {
        if (numneighbors >= MAX_WAREHOUSES) {
            throw runtime_error("Maximum number of neighbors reached for the warehouse.");
        }

        neighbors[numneighbors] = neighbor;
        distances[numneighbors] = distance;
        ++numneighbors;
    }

    void addproduct(const string& product) {
        if (numProducts >= MAX_PRODUCTS) {
            throw runtime_error("Maximum number of products reached for the warehouse.");
        }

        products[numProducts] = product;
        ++numProducts;
    }

    bool hasProduct(const string& product) const {
        for (int i = 0; i < numProducts; ++i) {
            if (products[i] == product) {
                return true;
            }
        }
        return false;
    }

    bool candeliverwithdrones(int distance) const {
        return Shortdistancedrone.cancompletedelivery(distance) || Longdistancedrone.cancompletedelivery(distance);
    }

    int calculatedeliverycharge(int distance) const {
        if (Shortdistancedrone.cancompletedelivery(distance)) {
            return Shortdistancedrone.getdeliverycharge(distance);
        } else if (Longdistancedrone.cancompletedelivery(distance)) {
            return Longdistancedrone.getdeliverycharge(distance);
        } else {
            throw runtime_error("No drone can complete the delivery to the required warehouse.");
        }
    }


    int dfscalculatedistance(Warehouse* destinationwarehouse, bool visited[], int accumulatedDistance) const {

        visited[id - 1] = true;


        if (this == destinationwarehouse) {
            return accumulatedDistance;
        }

        int mindistance = numeric_limits<int>::max();


        for (int i = 0; i < numneighbors; ++i) {
            Warehouse* neighbor = neighbors[i];
            if (!visited[neighbor->id - 1]) {
                int distanceToNeighbor = distances[i];
                int distanceToDestination = neighbor->dfscalculatedistance(destinationwarehouse, visited, accumulatedDistance + distanceToNeighbor);
                mindistance = min(mindistance, distanceToDestination);
            }
        }

        return mindistance;
    }


    int calculatedistance(Warehouse* destinationwarehouse) const {
        bool visited[MAX_WAREHOUSES] = {false};
        return dfscalculatedistance(destinationwarehouse, visited, 0);
    }
};


class DeliveryOrder {
public:
    Warehouse* sourcewarehouse;
    Warehouse* destinationwarehouse;
    string product;
    bool productAvailable;
    int distance;
    int deliveryCharge;

    DeliveryOrder(Warehouse* sourcewarehouse, Warehouse* destinationwarehouse, const string& product)
        : sourcewarehouse(sourcewarehouse),
          destinationwarehouse(destinationwarehouse),
          product(product),
          productAvailable(false),
          distance(0),
          deliveryCharge(0) {
        
        distance = sourcewarehouse->calculatedistance(destinationwarehouse);
    }

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


class Dronedeliverysystem {
private:
    Warehouse* warehouses[MAX_WAREHOUSES];
    int numWarehouses;
    DeliveryOrder* deliveryOrders[MAX_WAREHOUSES];
    int numdeliveryorders;


    Warehouse* findNearestWarehouseWithProduct(const string& product) {
        bool visited[MAX_WAREHOUSES] = {false};
        queue<Warehouse*> q;
        for (int i = 0; i < numWarehouses; ++i) {
            q.push(warehouses[i]);
            visited[warehouses[i]->id - 1] = true;
        }

        while (!q.empty()) {
            Warehouse* currentWarehouse = q.front();
            q.pop();

            if (currentWarehouse->hasProduct(product)) {
                return currentWarehouse;
            }

            for (int i = 0; i < currentWarehouse->numneighbors; ++i) {
                Warehouse* neighbor = currentWarehouse->neighbors[i];
                if (!visited[neighbor->id - 1]) {
                    visited[neighbor->id - 1] = true;
                    q.push(neighbor);
                }
            }
        }

        return nullptr;
    }

public:
    Dronedeliverysystem() : numWarehouses(0), numdeliveryorders(0) {}

    Warehouse* createwarehouse(int id, int shortDistanceBattery, int longDistanceBattery) {
        if (numWarehouses >= MAX_WAREHOUSES) {
            throw runtime_error("Maximum number of warehouses reached.");
        }

        Warehouse* warehouse = new Warehouse(id, shortDistanceBattery, longDistanceBattery);
        warehouses[numWarehouses] = warehouse;
        ++numWarehouses;
        return warehouse;
    }

    void addwarehouseneighbor(Warehouse* warehouse, Warehouse* neighbor, int distance) {
        warehouse->addneighbor(neighbor, distance);
    }

    void addwarehouseproduct(Warehouse* warehouse, const string& product) {
        warehouse->addproduct(product);
    }

    void createdeliveryorder(Warehouse* destinationwarehouse, const string& product) {
        if (numdeliveryorders >= MAX_WAREHOUSES) {
            throw runtime_error("Maximum number of delivery orders reached.");
        }


        Warehouse* sourcewarehouse = findNearestWarehouseWithProduct(product);

        if (sourcewarehouse != nullptr) {
            DeliveryOrder* deliveryOrder = new DeliveryOrder(sourcewarehouse, destinationwarehouse, product);
            deliveryOrder->checkProductAvailability();
            deliveryOrder->calculatedeliverycharge();
            deliveryOrders[numdeliveryorders] = deliveryOrder;
            ++numdeliveryorders;
        } else {
            cout << "Product '" << product << "' not available in any warehouse. Skipping delivery." << endl;
        }
    }

    void processdeliveries() {
        for (int i = 0; i < numdeliveryorders; ++i) {
            DeliveryOrder* deliveryOrder = deliveryOrders[i];
            deliveryOrder->printDeliveryDetails();
            cout << endl;
        }
    }

    ~Dronedeliverysystem() {
        for (int i = 0; i < numWarehouses; ++i) {
            delete warehouses[i];
        }

        for (int i = 0; i < numdeliveryorders; ++i) {
            delete deliveryOrders[i];
        }
    }
};

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
    } catch (const exception& ex) {
        cout << "An error occurred: " << ex.what() << endl;
    }

    return 0;
}
