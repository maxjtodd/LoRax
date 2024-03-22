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
    
    var body: some View {
            VStack {
                TabView {
                    ChatView()
                        .tabItem {
                            Image(systemName: "message.fill")
                            Text("Chat")
                        }
                    ContactView()
                        .tabItem {
                            Image(systemName: "person.fill")
                            Text("Contacts")
                        }
                    AdvancedView()
                        .tabItem {
                            Image(systemName: "gearshape.fill")
                            Text("Advanced")
                        }
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
    
    var body: some View {
        List(bluetoothManager.devices, id: \.self) { device in
            Text(device.name ?? "Unknown Device")
        }
        .onAppear {
            bluetoothManager.scanForDevices()
        }
    }
}

class BluetoothManager: NSObject, CBCentralManagerDelegate, ObservableObject {
    private var centralManager: CBCentralManager!
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
    
    func centralManager(_ central: CBCentralManager, didDiscover peripheral: CBPeripheral, advertisementData: [String: Any], rssi RSSI: NSNumber) {
        DispatchQueue.main.async { [weak self] in
            guard let self = self else { return }
            if !self.devices.contains(peripheral) {
                self.devices.append(peripheral)
            }
        }
    }
}


@available(iOS 16.0, *)
struct ContentView_Previews: PreviewProvider {
    static var previews: some View {
        ContentView().environment(\.managedObjectContext, PersistenceController.preview.container.viewContext)
    }
}
