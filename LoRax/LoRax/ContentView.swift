//
//  ContentView.swift
//  LoRax
//
//  Created by Max Todd on 2/26/24.
//

import SwiftUI
import CoreBluetooth


@available(iOS 16.0, *)
struct ContentView: View {
    
    @State private var tabSelection = 1
    
    var body: some View {
            VStack {
                TabView(selection:$tabSelection) {
                    ChatView()
                        .tabItem {
                            Image(systemName: "message.fill")
                            Text("Chat")
                        }
                        .tag(0)
                    ContactView()
                        .tabItem {
                            Image(systemName: "person.fill")
                            Text("Contacts")
                        }
                        .tag(1)
                    AdvancedView()
                        .tabItem {
                            Image(systemName: "gearshape.fill")
                            Text("Advanced")
                        }
                        .tag(2)
                }
            }
        }
    }




struct ChatView: View {
    
    var body: some View {
        Text("Placeholder for available devices")
    }
}


struct AdvancedView: View {
    @State private var showBluetoothDevices = false
    var body: some View {
        VStack {
            Button(action: {
                showBluetoothDevices.toggle()
            }) {
                Text("Show Bluetooth Devices")
                    .padding()
                    .background(Color.blue)
                    .foregroundColor(Color.white)
                    .cornerRadius(10)
            }
            .padding()
            Spacer()
            if showBluetoothDevices {
                BluetoothDevicesView()
            }
            Spacer()
        }
    }
}


struct BluetoothDevicesView: View {
    @StateObject var bluetoothManager = BluetoothManager()
    @State private var selectedDevice: CBPeripheral? = nil
    
    var body: some View {
        List(bluetoothManager.devices, id: \.self) { device in
            Button(action: {
                selectedDevice = device
                bluetoothManager.connect(peripheral: device)
            }) {
                Text(device.name ?? "Unknown Device")
            }
        }
        .onAppear {
            bluetoothManager.scanForDevices()
        }
    }
}

class BluetoothManager: NSObject, CBCentralManagerDelegate, ObservableObject {
    private var centralManager: CBCentralManager!
    private var connectedPeripheral: CBPeripheral?
    @Published var devices: [CBPeripheral] = []
    
    override init() {
        super.init()
        centralManager = CBCentralManager(delegate: self, queue: nil)
    }
    
    func scanForDevices() {
        centralManager.scanForPeripherals(withServices: nil, options: nil)
    }
    
    func centralManagerDidUpdateState(_ central: CBCentralManager) {
        if central.state == .poweredOn {
            scanForDevices()
        }
    }
    
    func connect(peripheral: CBPeripheral) {
        centralManager.connect(peripheral, options: nil)
    }
    
    func centralManager(_ central: CBCentralManager, didDiscover peripheral: CBPeripheral, advertisementData: [String: Any], rssi RSSI: NSNumber) {
        DispatchQueue.main.async { [weak self] in
            guard let self = self else { return }
            if !self.devices.contains(peripheral) {
                self.devices.append(peripheral)
            }
        }
    }
    
    func sendMessage(_ message: String) {
        guard let connectedPeripheral = connectedPeripheral else {
            print("Not connected to a device.")
            return
        }
        
        guard let data = message.data(using: .utf8) else {
            print("Message failed to convert.")
            return
        }
        
        connectedPeripheral.writeValue(data, for: <#Characteristic#>, type: .withoutResponse)
        // need to figure out what "Characteristic" needs to be
    }
}


@available(iOS 16.0, *)
struct ContentView_Previews: PreviewProvider {
    static var previews: some View {
        ContentView().environment(\.managedObjectContext, PersistenceController.preview.container.viewContext)
    }
}
