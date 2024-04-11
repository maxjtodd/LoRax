//
//  BluetoothService.swift
//  LoRax
//
//  Created by Ben Weber on 4/9/24.
//

import Foundation
import CoreBluetooth

enum ConnectionStatus {
    case connected
    case disconnected
    case scanning
    case connecting
    case error
}

let loraxServiceUUID = CBUUID(string: "B99CFDBD-4F69-42AA-82DA-68B92D310DEA")
let loraxTxCharacteristicUUID = CBUUID(string: "0F146B5F-2B7E-46A0-B246-36ED1867F6E7")
let loraxRxCharacteristicUUID = CBUUID(string: "B54E3121-477C-4A86-9FE7-19292CFA415B")

class BluetoothService: NSObject, ObservableObject {
    var viewModel: BluetoothViewModel!
    private var centralManager: CBCentralManager!
    var loraxPeripheral: CBPeripheral?
    @Published var peripheralStatus: ConnectionStatus = .disconnected
    
    override init() {
        super.init()
        viewModel = BluetoothViewModel()
        centralManager = CBCentralManager(delegate: self, queue: nil)
    }
    
    func scanForPeripherals() {
        peripheralStatus = .scanning
        centralManager.scanForPeripherals(withServices: [loraxServiceUUID])
    }
}

extension BluetoothService: CBCentralManagerDelegate {
    func centralManagerDidUpdateState(_ central: CBCentralManager) {
        if central.state == .poweredOn {
            print("CB Powered On")
            scanForPeripherals()
        }
    }
    
    func centralManager(_ central: CBCentralManager, didDiscover peripheral: CBPeripheral, advertisementData: [String : Any], rssi RSSI: NSNumber) {
        
        print("Discovered \(peripheral.name ?? "no name")")
        loraxPeripheral = peripheral
        centralManager.connect(loraxPeripheral!)
        peripheralStatus = .connecting
    }
    
    func centralManager(_ central: CBCentralManager, didConnect peripheral: CBPeripheral) {
        peripheralStatus = .connected
        peripheral.delegate = self
        peripheral.discoverServices([loraxServiceUUID])
        centralManager.stopScan()
    }
    
    func centralManager(_ central: CBCentralManager, didDisconnectPeripheral peripheral: CBPeripheral, error: (any Error)?) {
        peripheralStatus = .disconnected
    }
    
    func centralManager(_ central: CBCentralManager, didFailToConnect peripheral: CBPeripheral, error: (any Error)?) {
        peripheralStatus = .error
        print(error?.localizedDescription ?? "no error")
    }
}

extension BluetoothService: CBPeripheralDelegate {
    func peripheral(_ peripheral: CBPeripheral, didDiscoverServices error: (any Error)?) {
        for service in peripheral.services ?? [] {
            if service.uuid == loraxServiceUUID {
                peripheral.discoverCharacteristics([loraxTxCharacteristicUUID], for: service)
            }
        }
    }
    
    func peripheral(_ peripheral: CBPeripheral, didDiscoverCharacteristicsFor service: CBService, error: (any Error)?) {
        for characteristic in service.characteristics ?? [] {
            peripheral.setNotifyValue(true, for: characteristic)
        }
    }
    
    func peripheral(_ peripheral: CBPeripheral, didUpdateValueFor characteristic: CBCharacteristic, error: (any Error)?) {
        if characteristic.uuid == loraxTxCharacteristicUUID {
            guard let data = characteristic.value else {
                print("No data received for \(characteristic.uuid.uuidString)")
                return
            }
            print(String(data: data, encoding: .utf8) ?? "unknown string")
            didReceiveMessage(data: data)
        }
    }
    
    func didReceiveMessage(data: Data) {
        let receivedMessage = String(data: data, encoding: .utf8)!
        viewModel!.updateReceivedMessage(newMessage: receivedMessage)
            
    }
}

// ViewModel to hold the text to be displayed
class BluetoothViewModel: ObservableObject {
    @Published var receivedMessage: String = ""
    
    // Function to update the received text
    func updateReceivedMessage(newMessage: String) {
        print("update got message: " + newMessage)
        receivedMessage = newMessage
    }
}
