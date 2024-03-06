//
//  Message.swift
//  LoRax
//
//  Created by Max Todd on 3/1/24.
//

import Foundation



struct Message {
    var content: String
    var date: String = Date().formatted(date: .numeric, time: .omitted)
    var time: String = Date().formatted(date: .omitted, time: .shortened)
}

func recieveMessage(m: Message) {
    print("Recieving: \(m.content)")
}

func sendMessage(m: Message) {
    print("Sending: \(m.content)")
}

