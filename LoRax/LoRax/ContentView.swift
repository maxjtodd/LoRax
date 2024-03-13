//
//  ContentView.swift
//  LoRax
//
//  Created by Max Todd on 2/26/24.
//

import SwiftUI

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
                    ContactsView()
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

@available(iOS 16.0, *)
struct ContactsView: View {
    var body: some View {
        Text ("This is the contacts page.")
    }
}

@available(iOS 16.0, *)
struct ChatView: View {
    @State private var chatMessages: [String] = [
            "Hey, everyone!",
            "This is the LoRax app",
            "Hey, everyone!",
            "This is the LoRax app","Hey, everyone!",
            "This is the LoRax app","Hey, everyone!",
            "This is the LoRax app","Hey, everyone!",
            "This is the LoRax app","Hey, everyone!",
            "This is the LoRax app","Hey, everyone!",
            "This is the LoRax app","Hey, everyone!",
            "This is the LoRax app",
        ]
        
    var body: some View {
        ScrollView {
            VStack(spacing: 10) {
                ForEach(chatMessages, id: \.self) { message in
                    Text(message)
                        .padding()
                        .background(Color.blue)
                        .foregroundColor(.white)
                        .cornerRadius(10)
                        .frame(maxWidth: .infinity, alignment: .leading)
                }
            }
            .padding()
        }
    }
}

@available(iOS 16.0, *)
struct AdvancedView: View {
    var body: some View {
        Text ("This is the advanced page.")
    }
}

@available(iOS 16.0, *)
struct ContentView_Previews: PreviewProvider {
    static var previews: some View {
        ContentView()
    }
}
