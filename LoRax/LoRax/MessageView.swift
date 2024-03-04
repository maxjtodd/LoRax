//
//  MessageView.swift
//  LoRax
//
//  Created by Max Todd on 3/1/24.
//

import SwiftUI

struct MessageView: View {
    var body: some View {
//        OneMessageView(message: <#T##Message#>)
        VStack {
        OneMessageRecieved(message: Message(content: "message recieved"))
        OneMessageSentView(message: Message(content: "message sent"))
        }
    }
}

struct OneMessageSentView: View {
    
    // Stores the message for
    var message: Message
    
    var body: some View {
        
        // date \ message HStack
        VStack () {
            
            // message   time
            HStack {
                Text(message.content)
                    .padding(.horizontal, 10.0)
                    .background(.blue)
                    .frame(width: UIScreen.main.bounds.size.width / 1.5, alignment: .bottomTrailing)
                Spacer()
                Text(message.time)
                    .padding(.horizontal, 10.0)
                    .frame(width: UIScreen.main.bounds.size.width / 3.5, alignment: .bottomTrailing)
            }
            .padding([.top, .leading, .bottom], 10)
            
        }.padding(0)
        
        
    }
}

struct OneMessageRecieved: View {
    
    // Stores the message for
    var message: Message
    
    var body: some View {
        
        // date \ message HStack
        VStack () {
            
            // message   time
            HStack {
                Text(message.content)
                    .padding(.horizontal, 10.0)
                    .background(.gray)
                    .frame(width: UIScreen.main.bounds.size.width / 1.5, alignment: .leading)
//                    .cornerRadius(10)
                Spacer()
                Text(message.time)
                    .padding(.horizontal, 10.0)
                    .frame(width: UIScreen.main.bounds.size.width / 3.5, alignment: .bottomTrailing)
            }
            .padding([.top, .leading, .bottom], 10)
            
        }.padding(0)
        
        
    }
}

struct MessageView_Previews: PreviewProvider {
    static var previews: some View {
        MessageView()
    }
}

