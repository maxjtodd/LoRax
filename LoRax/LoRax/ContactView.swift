//
//  ContactView.swift
//  TestCoreData
//
//  Created by Max Todd on 3/20/24.
//

import SwiftUI
import CoreData

@available(iOS 16.0, *)
struct ContactView: View {
    
    // context to create, edit, and modify core data objects
    @Environment(\.managedObjectContext) private var viewContext
    
    
    @FetchRequest(
        sortDescriptors: [
            NSSortDescriptor(keyPath: \Contact.mac, ascending: true)
        ],
        animation: .default)
    private var items: FetchedResults<Contact>
    
    @State private var multiSelection = Set<UUID>()
    
    // Properties for adding a new contact
    @State private var addingContact = false
    @State private var fName: String = ""
    @State private var lName: String = ""
    @State private var mac: String = ""
    
    var body: some View {
        
        // Display contact list
        if (!addingContact) {
            NavigationView {
                
                // Display the contacts if not adding contact
                
                List(items, selection: $multiSelection) {
                    ContactListSlot(contact: $0)
                }
                .navigationTitle("Contacts")
                .toolbar {
                    ToolbarItemGroup(placement: .topBarLeading) {
                        Spacer()
                        Button("", systemImage: "plus") {
                            print("Pressed")
                            addingContact = true
                        }
                        Spacer()
                        
                    }
                    
                    ToolbarItemGroup(placement: .topBarTrailing) {
                        Spacer()
                        EditButton()
                        Spacer()
                        
                    }
                }
                
            }.navigationBarHidden(true)
            
        }
        
        // Display adding contact
        else {
            
            NavigationView {
                VStack {
                    
                    TextField(
                        "First Name",
                        text: $fName
                    )
                        
                    TextField(
                        "Last Name",
                        text: $lName
                    )
                        
                    
                    TextField(
                        "MAC",
                        text: $lName
                    )
                        
                    Button("Add") {
                        print("Pressed")
                    }
                    
                    
                }
                .navigationTitle("Add Contact")
                .toolbar {
                    
                    ToolbarItemGroup(placement: .topBarTrailing) {
                        Spacer()
                        Button("Cancel") {
                            print("Pressed")
                            addingContact = false
                        }
                        Spacer()
                        
                    }
                }
            }
            .navigationBarHidden(true)
        }
    }
        
}


@available(iOS 16.0, *)
struct ContactListSlot: View {
    
    // Stores the contact
    var contact: Contact
    
    var body: some View {
        
        HStack {
            
            NavigationLink(destination: MessageView(currentMac: contact.mac!).toolbar(.hidden, for: .tabBar)) {
                Text("\(contact.lName!), \(contact.fName!)    -> \(contact.mac!)")
            }
            
            
        }
        
    }
    
}


@available(iOS 16.0, *)
struct ContactView_Previews: PreviewProvider {
    static var previews: some View {
        ContactView().environment(\.managedObjectContext, PersistenceController.preview.container.viewContext)
    }
}
