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
    
    // get a list of all non contacts
    @FetchRequest(
        sortDescriptors: [
            NSSortDescriptor(keyPath: \NonContact.mac, ascending: true)
        ],
        animation: .default)
    private var nonContacts: FetchedResults<NonContact>
    
    // stores the selected components in edit mode
    @State private var multiSelection = Set<UUID>()
    
    // Properties for adding a new contact
    @State private var addingContact = false
    @State private var fName: String = ""
    @State private var lName: String = ""
    @State private var mac: String = ""
    
    var body: some View {
        
        // Display the contacts if not adding contact
        NavigationView {
            
            if (!addingContact) {
                
                List {
                    
                    // Display Contacts
                    ForEach(self.contacts) { c in
                        ContactListSlot(contact: c)
                    }
                    .onDelete(perform: deleteContactAtIndex)
                    
                    // Display Non Contacts
                    Section(header: Text("Non-Contacts")) {
                        ForEach(self.nonContacts) { nc in
                            nonContactListSlot(nonContact: nc)
                                .swipeActions(edge: /*@START_MENU_TOKEN@*/.trailing/*@END_MENU_TOKEN@*/) {
                                    
                                    Button {createFromNonContact(nonContact: nc)} label: {
                                        Label("Add Contact", systemImage: "plus.circle")
                                    }
                                        .tint(.green)
                                }
                        }

                    }
                }
                .navigationTitle("Contacts")
                .toolbar {
                    ToolbarItemGroup(placement: .topBarLeading) {
                        Spacer()
                        Button("", systemImage: "plus") {
                            fName = ""
                            lName = ""
                            mac = ""
                            addingContact = true
                            print(addingContact)
                        }
                        Spacer()
                        
                    }
                    
                    ToolbarItemGroup(placement: .topBarTrailing) {
                        Spacer()
                        EditButton()
                        Spacer()
                        
                    }
                }
                
                
                
            }
            
            else {
                VStack {
                    
                    TextField("First Name", text: $fName)
                        .multilineTextAlignment(.center)
                        .padding()
                    TextField("Last Name", text: $lName)
                        .multilineTextAlignment(.center)
                        .padding()
                    TextField("MAC",text: $mac)
                        .multilineTextAlignment(.center)
                        .padding()
                    
                    Button("Add") {
                        addContact(fName: fName, lName: lName, mac: mac)
                        addingContact = false
                    }
                        .padding()
                    
                    
                }
                .navigationTitle("Add Contact")
                .toolbar {
                    
                    ToolbarItemGroup(placement: .topBarTrailing) {
                        Spacer()
                        Button("Cancel") {
                            addingContact = false
                            print(addingContact)
                        }
                        Spacer()
                        
                    }
                }
            }
            
        }.navigationBarHidden(true)
    }
    
    
    
    /// Add a contact permanently into core data
    /// - Parameters:
    ///   - fName: first name of the contact to add
    ///   - lName: last name of the contact to add
    ///   - mac: mac address of the contact to add
    private func addContact(fName: String, lName: String, mac: String) {
        
        // TODO: Delete non contact
        // Delete non contact, adding as a new contact
        for nc in nonContacts {
            if nc.mac == mac {
                deleteNonContact(nonContact: nc)
            }
        }
        
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
    
    
    /// Delete a contact permanently from core data
    /// - Parameter contact: Contact to delete
    private func deleteContact(contact: Contact) {
        withAnimation {
            viewContext.delete(contact)

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
    
    
    /// Delete a contact permanently from core data given index in fetch request
    /// - Parameter contact: Contact to delete
    private func deleteContactAtIndex(offsets: IndexSet) {
        
        for i in offsets {
            let contact: Contact = contacts[i]
            deleteContact(contact: contact)
        }
        
    }
    
    
    /// Delete non contact permanently from core data
    /// - Parameter nonContact: nonContact to delete
    private func deleteNonContact(nonContact: NonContact) {
        withAnimation {
            viewContext.delete(nonContact)

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
    
    
    private func createFromNonContact(nonContact: NonContact) {
        
        // Switch to add contact page and autofil mac address
        addingContact = true
        mac = nonContact.mac!
        print(mac)
        
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
struct nonContactListSlot: View {
    
    // Stores the contact
    var nonContact: NonContact
    
    var body: some View {
        HStack {
            NavigationLink(destination: MessageView(currentMac: nonContact.mac!).toolbar(.hidden, for: .tabBar)) {
                Text("\(nonContact.mac!)")
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
