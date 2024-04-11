//
//  ContentView.swift
//  LoRax
//
//  Created by Max Todd on 2/26/24.
//

import SwiftUI
import CoreBluetooth
import Foundation

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
    @State private var showBluetoothMessages = false
    @StateObject private var bluetoothViewModel = BluetoothViewModel()
    @StateObject private var bluetoothService = BluetoothService()
    
    var body: some View {
        VStack {
            Button(action: {
                showBluetoothMessages.toggle()
            }) {
                Text("Show Bluetooth Messages")
                    .padding()
                    .background(Color.blue)
                    .foregroundColor(Color.white)
                    .cornerRadius(10)
            }
            .padding()
            Spacer()
            if showBluetoothMessages {
                BluetoothMessagesView()
                    .environmentObject(bluetoothViewModel)
                    .environmentObject(bluetoothService)
            }
            Spacer()
        }
    }
}


struct BluetoothMessagesView: View {
    @EnvironmentObject var bluetoothViewModel: BluetoothViewModel
    @EnvironmentObject var bluetoothService: BluetoothService
    
    var body: some View {
        VStack {
            Text("Received Text: \(bluetoothViewModel.receivedMessage)")
            .padding()
        
        }
    }
}

@available(iOS 16.0, *)
struct ContentView_Previews: PreviewProvider {
    static var previews: some View {
        ContentView().environment(\.managedObjectContext, PersistenceController.preview.container.viewContext)
    }
}
