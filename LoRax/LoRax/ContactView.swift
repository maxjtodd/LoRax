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
    
    // get a list of all contacts
    @FetchRequest(
        sortDescriptors: [
            NSSortDescriptor(keyPath: \Contact.mac, ascending: true)
        ],
        animation: .default)
    private var contacts: FetchedResults<Contact>
    
    // stores the selected components in edit mode
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
                
                List(contacts, selection: $multiSelection) {
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
                
                //TODO: List all message history of non-contacts
                
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
                        text: $mac
                    )
                        
                    Button("Add") {
                        addContact(fName: fName, lName: lName, mac: mac)
                        addingContact = false
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
    
    
    
    /// Add a contact permanently into core data
    /// - Parameters:
    ///   - fName: first name of the contact to add
    ///   - lName: last name of the contact to add
    ///   - mac: mac address of the contact to add
    private func addContact(fName: String, lName: String, mac: String) {
        
        withAnimation{
            
            // Create the new contact
            let newContact = Contact(context: viewContext)
            newContact.fName = fName
            newContact.lName = lName
            newContact.mac = mac
            
            // Save the contact
            do {
                try viewContext.save()
            } catch {
                // Replace this implementation with code to handle the error appropriately.
                // fatalError() causes the application to generate a crash log and terminate. You should not use this function in a shipping application, although it may be useful during development.
                let nsError = error as NSError
                fatalError("Unresolved error \(nsError), \(nsError.userInfo)")
            }
            
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
