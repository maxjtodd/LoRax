//
//  ChatHistoryView.swift
//  LoRax
//
//  Created by Max Todd on 4/29/24.
//

import SwiftUI
import CoreData

struct ChatView: View {
    
    // context to create, edit, and modify core data objects
    @Environment(\.managedObjectContext) private var viewContext
    
    // get a list of all messages
    @FetchRequest(
        sortDescriptors: [
            NSSortDescriptor(keyPath: \Contact.lName, ascending: true)
        ],
        animation: .default)
    private var contacts: FetchedResults<Contact>
    
    var body: some View {
        
        NavigationView {
            List {
                
                // Chat Now
                // TODO: implement when hardware supports
                Section(header: Text("Available")) {
                    Text("Placeholder")
                }
                
                // Message History
                Section(header: Text("History")) {
                    lastMessageView(macs: self.getMessageMacs())
                }
            }
                .navigationTitle("Chat")
        }
        
        
    }
    
    /// Get the 5 latest message senders/recievers
    /// - Returns: String array of the mac addresses of the last communicated with devices
    private func getMessageMacs() -> [String] {
        
        // Get the last 5 messages from unique senders sent with core data. Only get last 50,
        //  if there is any more after that only display less contacts.
        let request: NSFetchRequest<Message> = Message.fetchRequest()
        request.fetchLimit = 50
        request.sortDescriptors = [NSSortDescriptor(keyPath: \Message.date, ascending: false)]
        
        // Try to get the messages
        do {
            let messages = try viewContext.fetch(request)
            if messages.count == 0 {
                return []
            }
            
            // Get the latest messages with unique contacts
            var contacts: [String] = []
            for m in messages {
                
                // Get the contact, add to contacts if unique
                let c = m.mac
                var found = false
                contacts.indices.forEach { i in
                    if contacts[i] == c! {
                        found = true
                    }
                }
                
                if !found {
                    contacts.append(c!)
                }
                
            }
            
            // Return all of the MAC addresses of the latest messages sent
            return contacts
            
        }
        // Error occured fetching messages
        catch {
            print("Latest Message Get Error")
            return []
        }
    }
}

//#Preview {
//    ChatHistoryView()
//}

@available(iOS 16.0, *)
struct ChatHistoryView_Previews: PreviewProvider {
    static var previews: some View {
        ChatView().environment(\.managedObjectContext, PersistenceController.preview.container.viewContext)
    }
}
