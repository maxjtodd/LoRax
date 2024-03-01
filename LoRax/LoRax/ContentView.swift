//
//  ContentView.swift
//  LoRax
//
//  Created by Max Todd on 2/26/24.
//

import SwiftUI

struct ContentView: View {
    @State private var message: String = ""
    
    var body: some View {
        VStack {
            TextField("Enter your message", text: $message)
                .padding()
                .textFieldStyle(RoundedBorderTextFieldStyle())
                .padding()
            
            Spacer()
            
            HStack {
                Button(action: {
                    print("Chat button pushed")
                }) {
                    Text("Chat")
                        .padding()
                        .foregroundColor(.white)
                        .background(Color.gray)
                        .cornerRadius(10)
                }
                
                Button(action: {
                    print("Advanced button pushed")
                }) {
                    Text("Advanced")
                        .padding()
                        .foregroundColor(.white)
                        .background(Color.gray)
                        .cornerRadius(10)
                }
            }
            .padding()
        }
        .padding()
    }
}

struct ContentView_Previews: PreviewProvider {
    static var previews: some View {
        ContentView()
    }
}
