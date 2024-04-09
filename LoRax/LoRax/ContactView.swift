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
            NSSortDescriptor(keyPath: \Contact.lName, ascending: true)
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
    
    // Properties for deleting contact
    @State private var deletingContact = false
    @State private var deletingMac: String = ""
    @State private var deletingFName: String = ""
    @State private var deletingLName: String = ""
    
    // Property for modifying contact
    @State private var modifyingContact = false
    
    // UI
    var body: some View {
        
        // Display the contacts if not adding contact
        NavigationView {
            
            // Not adding contact, display regular contact screen
            if (!addingContact && !modifyingContact) {
                
                List {
                    
                    // Display Contacts
                    ForEach(self.contacts) { c in
                        ContactListSlot(contact: c)
                        
                            // On Swipe Actions
                            .swipeActions(edge: /*@START_MENU_TOKEN@*/.trailing/*@END_MENU_TOKEN@*/) {
                                
                                // Delete Contact
                                Button(role: .destructive) {
                                    deletingContact = true
                                    deletingMac = c.mac!
                                    deletingFName = c.fName!
                                    deletingLName = c.lName!
                                    self.deleteContact(contact: c)
                                } label: {
                                    Label("Delete Contact", systemImage: "trash")
                                }
                                .tint(.red)
                                
                                
                                // Modify Contact
                                Button() {
                                    fName = c.fName!
                                    lName = c.lName!
                                    mac = c.mac!
                                    modifyingContact = true
                                } label: {
                                    Label("Modify Contact", systemImage: "gear")
                                }
                                .tint(.orange)
                            }
                        
                            // Confirm message deletion
                            .confirmationDialog(
                                Text("Delete contact data?"),
                                isPresented: $deletingContact,
                                titleVisibility: .visible
                            ) {
                                Button("Delete All Chat History", role: .destructive) {
                                    deleteMessagesFromMac(mac: deletingMac)
                                }
                                Button("Cancel data deletion", role: .destructive) {
                                    _ = createNonContact(mac: deletingMac)
                                }
                                Button("Cancel", role: .cancel) {
                                    _ = addContact(fName: deletingFName, lName: deletingLName, mac: deletingMac)
                                }
                            }
                        
                    }
                    
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
                // Toolbar
                .navigationTitle("Contacts")
                .toolbar {
                    ToolbarItemGroup(placement: .topBarLeading) {
                        Spacer()
                        Button("", systemImage: "minus") {
                            let v = validateMac(mac: "00:1A:2B:3C:4D:5E")
                            print(v)
                        }
                        Spacer()
                        
                    }
                    
                    ToolbarItemGroup(placement: .topBarTrailing) {
                        Spacer()
                        Button("", systemImage: "plus") {
                            fName = ""
                            lName = ""
                            mac = ""
                            addingContact = true
//                            print(addingContact)
                        }
                        Spacer()
                        
                    }
                }
            }
            
            // Adding contact, display adding contact screen
            else if (addingContact) {
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
                        let added = addContact(fName: fName, lName: lName, mac: mac)
                        print("added: \(added)")
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
                            fName = ""
                            lName = ""
                            mac = ""
//                            print(addingContact)
                        }
                        Spacer()
                        
                    }
                }
            }
            
            // Modifying contact, display editing contact screen
            else {
                
                VStack {
                    
                    TextField("First Name", text: $fName)
                        .multilineTextAlignment(.center)
                        .padding()
                    TextField("Last Name", text: $lName)
                        .multilineTextAlignment(.center)
                        .padding()
                    Text(mac)
                        .padding()
                        .foregroundColor(.gray)
                    
                    Button("Modify") {

                        
                        modifyingContact = false
                    }
                        .padding()
                    
                    
                }
                .navigationTitle("Modify Contact")
                .toolbar {
                    
                    ToolbarItemGroup(placement: .topBarTrailing) {
                        Spacer()
                        Button("Cancel") {
                            modifyingContact = false
                            fName = ""
                            lName = ""
                            mac = ""
//                            print(addingContact)
                        }
                        Spacer()
                        
                    }
                }
                
            }
            
        }.navigationBarHidden(true)
    }
    
    
    
    /// Determine if the inputted mac add
    /// - Parameter mac: Mac address to validate
    /// - Returns: True if mac address is valid, false if not
    private func validateMac(mac: String) -> Bool {
        let macRegex = "^([0-9A-Fa-f]{2}[:-]){5}([0-9A-Fa-f]{2})$"
        let macTest = NSPredicate(format: "SELF MATCHES %@", macRegex)
        return macTest.evaluate(with: mac)
    }
    
    
    /// Add a contact permanently into core data
    /// - Parameters:
    ///   - fName: first name of the contact to add
    ///   - lName: last name of the contact to add
    ///   - mac: mac address of the contact to add
    private func addContact(fName: String, lName: String, mac: String) -> Bool {
        
        // Validate mac address
        let valid = validateMac(mac: mac)
        if !valid {
            print("Not adding... MAC not valid.")
            return false
        }
        
        // Determine if the mac is being used by another contact
        let request: NSFetchRequest<Contact> = Contact.fetchRequest()
        request.fetchLimit = 1
        request.predicate = NSPredicate(format: "mac LIKE %@", mac)
                
        // Send fetch request to see if mac exists
        do {
            
            // Mac exists, don't add
            let c = try viewContext.fetch(request).first
            if c != nil {
                print("At least one contact with mac \(mac)")
                return false
            }
            
        }
        // Error
        catch {
            print("Error adding contact - couldn't fetch")
            return false
        }
        
        // Delete non contact if non contact exists with that mac, adding as a new contact
        for nc in nonContacts {
            if nc.mac == mac {
                deleteNonContact(nonContact: nc)
            }
        }
        
        // Set up returning variable
        var returner = false
        withAnimation {
            
            // Create the new contact
            let newContact = Contact(context: viewContext)
            newContact.fName = fName
            newContact.lName = lName
            newContact.mac = mac
            newContact.id = UUID()
            
            // Save the contact
            do {
                try viewContext.save()
                returner = true
            }
            // contact didn't save
            catch {
                print("Error adding contact - couldn't save")
                returner = false
            }
            
        }
        return returner
        
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
    
    
    /// Modify a contact with the new parameters
    ///  Creates new non-contact if changing to a mac that isn't used
    /// - Parameters:
    ///   - mac: Mac of the contact to modify
    ///   - fName: New first name to set
    ///   - lName: New last name to set
    private func modifyContact(mac: String, fName: String, lName: String) {
        
        // Get the contact of the
    }
    
    
    /// Delete a contact permanently from core data given index in fetch request
    /// - Parameter contact: Contact to delete
    private func deleteContactAtIndex(offsets: IndexSet) {
        
        for i in offsets {
            let contact: Contact = contacts[i]
            deleteContact(contact: contact)
        }
        
    }
    
            
    /// Add a new non-contact
    /// - Parameter mac: Mac of the new non contact
    /// - Returns: Success of addition
    private func createNonContact(mac: String) -> Bool {
        
        // Validate mac address
        let valid = validateMac(mac: mac)
        if !valid {
            print("Not adding... MAC not valid.")
            return false
        }
        
        // Determine if the mac is being used by another contact
        let request: NSFetchRequest<NonContact> = NonContact.fetchRequest()
        request.fetchLimit = 1
        request.predicate = NSPredicate(format: "mac LIKE %@", mac)
                
        // Send fetch request to see if mac exists
        do {
            
            // Mac exists, don't add
            let nc = try viewContext.fetch(request).first
            if nc != nil {
                print("At least one contact with mac \(mac)")
                return false
            }
            
        }
        // Error
        catch {
            print("ERROR createNonContact")
        }
        
        // Set up return values
        var returner = false
        withAnimation {
            
            // Create the new non contact
            let newNonContact = NonContact(context: viewContext)
            let stringy: String
            stringy = mac
            newNonContact.mac = stringy
            newNonContact.id = UUID()
            
            // Save the non contact
            do {
                try viewContext.save()
                returner = true
            }
            // non contact didn't save
            catch {
                print("Error adding contact - couldn't save")
                returner = false
            }
        }
        
        return returner
        
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
    
    
    /// Start creating contact from a non contact message history
    /// - Parameter nonContact: Non contact to start creating contact for
    private func createFromNonContact(nonContact: NonContact) {
        
        // Switch to add contact page and autofil mac address
        addingContact = true
        fName = ""
        lName = ""
        mac = nonContact.mac!
        
    }
    
    
    /// Delete inputted message
    /// - Parameter message: Message to be deleted
    private func deleteMessage(message: Message) {
        
        viewContext.delete(message)

        do {
            try viewContext.save()
        } catch {
            // Replace this implementation with code to handle the error appropriately.
            // fatalError() causes the application to generate a crash log and terminate. You should not use this function in a shipping application, although it may be useful during development.
            let nsError = error as NSError
            fatalError("Unresolved error \(nsError), \(nsError.userInfo)")
        }
        
    }
    
    
    /// Delete all messages from a contact, used when deleting a contact
    /// - Parameter mac: Mac of the contact to be deleted
    private func deleteMessagesFromMac(mac: String) {
        
        let request: NSFetchRequest<Message> = Message.fetchRequest()
        request.predicate = NSPredicate(format: "mac LIKE %@", mac)
        
        do {
            let messages = try viewContext.fetch(request)
            for m in messages {
               deleteMessage(message: m)
            }
        } catch {
            let nsError = error as NSError
            fatalError("Unresolved error \(nsError), \(nsError.userInfo)")
        }
    }
    
}


@available(iOS 16.0, *)
/// UI list to navigate to the chat history for the given contact
struct ContactListSlot: View {
    
    // Stores the contact
    var contact: Contact
    
    var body: some View {
        if contact.mac != nil {
            HStack {
                NavigationLink(destination: MessageView(currentMac: contact.mac!).toolbar(.hidden, for: .tabBar)) {
                    Text("\(contact.lName!), \(contact.fName!)    -> \(contact.mac!)")
                }
            }
        }
        else {
            HStack{}
        }
    }
}


@available(iOS 16.0, *)
/// UI list to navigate to the chat history for the given non contact
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
