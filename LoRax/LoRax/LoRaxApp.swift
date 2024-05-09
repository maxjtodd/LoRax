//
//  LoRaxApp.swift
//  LoRax
//
//  Created by Max Todd on 2/26/24.
//

import SwiftUI

@available(iOS 16.0, *)
@main
struct LoRaxApp: App {
    let persistenceController = PersistenceController.shared
    
    
    
    var body: some Scene {
        WindowGroup {
            ContentView()
                .environment(\.managedObjectContext, persistenceController.container.viewContext)
        }
    }
}
