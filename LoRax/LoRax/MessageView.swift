//
//  MessageView.swift
//  TestCoreData
//
//  Created by Max Todd on 3/15/24.
//

import SwiftUI
import CoreData

@available(iOS 16.0, *)
struct MessageView: View {
    
    // context to create, edit, and modify core data objects
    @Environment(\.managedObjectContext) private var viewContext
    
    // Current mac address we chatting / viewing
    var currentMac:String = "1:1:1:1"
    
    // Get all messages sorted by ascending date
    // TODO: only for the mac address used
    @FetchRequest(
        sortDescriptors: [
            NSSortDescriptor(keyPath: \Message.date, ascending: true)
        ],
        animation: .default)
    private var messages: FetchedResults<Message>
    
    // get a list of all contacts
    @FetchRequest(
        sortDescriptors: [
            NSSortDescriptor(keyPath: \Contact.mac, ascending: true)
        ],
        animation: .default)
    private var contacts: FetchedResults<Contact>
    
    // State variable: what is stored in the Message... box
    @State private var messageBoxContent:String = ""
    
    // Stores the id of the last message in the scroll view to scroll to
    @State var messageIdToSetVisible: UUID?
    
    // Edit mode for deleting messages
    @State var editMode = false
    
    @State var deleteAllShowConfirmation = false
    

    var body: some View {

        NavigationView{
            VStack {
                
                ScrollViewReader { scrollProxy in
                    
                    ScrollView {
                        
                        ForEach(self.messages) { m in
                            HStack {
                                if (editMode) {
                                    if m.mac == currentMac {
                                        Spacer()
                                        Button("", systemImage: "minus.circle") {
                                            deleteMessage(message: m)
                                            print("Pressed")
                                        }
                                        Spacer()
                                        MessageComponentView(message: m, mac: currentMac, showDate: false)
                                    }
                                }
                                else {
                                    MessageComponentView(message: m, mac: currentMac)
                                }
                            }
                                .id(m.id)
                        }
                        
                    }
                    .onChange(of: self.messageIdToSetVisible) { id in
                        withAnimation {
                            scrollProxy.scrollTo(id)
                        }
                    }
                    .onAppear {
                        scrollProxy.scrollTo(self.messages.last?.id)
                    }
                    
                }
                
                // Bottom row
                // Not in edit mode: text input, coordinate send
                if !editMode {
                    
                    HStack {
                        // Display the chat box
                        TextField("Message...", text: $messageBoxContent)
                            .padding(.horizontal, 30)
                            .onSubmit {
                                addMessage(recieved: false)
                                messageBoxContent = ""
                                print("Adding")
                            }
                        
                        // Button to send coordinates
                        Button("", systemImage: "location.circle", action: {addMessage(recieved: false, location: true)})
                            .padding(.horizontal, 10)
                        
                        // Test button to recieve a message
                        Button("", systemImage: "arrowshape.left", action: {addMessage(recieved: true, content: "recieved message")})
                            .padding(.horizontal, 10)
                        
                    }
                }
                // Not in edit mode: delete all button
                else {
                    HStack {
                        
                        Spacer()
                        Button("Delete All") {
                            print("Delete All")
                            deleteAllShowConfirmation = true
                        }
                        .confirmationDialog("Are you sure you want to delete every message?",
                                            isPresented: $deleteAllShowConfirmation,
                                            titleVisibility: .visible
                        ) {
                            Button("Yes", role: .destructive) {
                                print("Delete All")
                                deleteAllMessages(messages: self.messages, mac: currentMac)
                            }
                        }
                        Spacer()
                    }
                }
                
                
            }
            // Navigation bar
            .navigationTitle(getTitle(mac: currentMac, myContacts: contacts))
            .navigationBarTitleDisplayMode(.inline)
            .toolbar {
                
                // Backwards Navigation
                ToolbarItemGroup(placement: .topBarLeading) {
                    Spacer()

                    NavigationLink(destination: ContentView()) {
                        Label("Back", systemImage: "arrowshape.backward")
                    }
                    
                    Spacer()
                    
                }
                
                // TODO: delete messages
                // TODO: animation
                ToolbarItemGroup(placement: .topBarTrailing) {
                    Spacer()
                    if !editMode {
                        Button("", systemImage: "square.and.pencil") {
                            editMode = !editMode
                        }
                    }
                    else {
                        Button("Done") {
                            editMode = !editMode
                        }
                    }
                    Spacer()
                    
                }
                    
            }
            
        }
            .navigationBarHidden(true)
        
    }


    /// Add a message to the database \
    /// - Parameters:
    ///   - recieved: Was the message recieved, boolean
    ///   - location: The message being sent is location data
    ///   - content: Content of the message. Gets updated with state variable $messageBoxContent if recieved = false
    private func addMessage(recieved: Bool, location:Bool = false, content: String="default") {
        
        withAnimation {
            
            // Create the new message
            let newMessage = Message(context: viewContext)
            newMessage.date = Date()
            newMessage.id = UUID()
            newMessage.content = content
            newMessage.recieved = recieved
            newMessage.mac = currentMac
            
            // Update the value of the message if the user is sending, pull from the state variable messageBoxContent
            if !newMessage.recieved {
                newMessage.content = messageBoxContent
            }
            
            // TODO Location data
            if location {
                newMessage.content = "location data.."
            }

            // Save the message
            do {
                try viewContext.save()
                messageIdToSetVisible = newMessage.id
            } catch {
                // Replace this implementation with code to handle the error appropriately.
                // fatalError() causes the application to generate a crash log and terminate. You should not use this function in a shipping application, although it may be useful during development.
                let nsError = error as NSError
                fatalError("Unresolved error \(nsError), \(nsError.userInfo)")
            }
        }
    }

//    private func deleteItems(offsets: IndexSet) {
//        withAnimation {
//            offsets.map { items[$0] }.forEach(viewContext.delete)
//
//            do {
//                try viewContext.save()
//            } catch {
//                // Replace this implementation with code to handle the error appropriately.
//                // fatalError() causes the application to generate a crash log and terminate. You should not use this function in a shipping application, although it may be useful during development.
//                let nsError = error as NSError
//                fatalError("Unresolved error \(nsError), \(nsError.userInfo)")
//            }
//        }
//    }
    
//    private func deleteMessage(offsets: IndexSet) {
    private func deleteMessage(message: Message) {
        withAnimation {
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
    }
    
    private func deleteAllMessages(messages: FetchedResults<Message>, mac: String) {
        withAnimation {
            
            for m in messages {
                if m.mac == mac {
                    deleteMessage(message: m)
                }
            }
            
        }
    }
    
    
    
    private func getTitle(mac: String, myContacts: FetchedResults<Contact>) -> String {
        
        for c in myContacts {
            if mac == c.mac {
                return "\(c.fName!) \(c.lName!)"
            }
        }
        
        return mac
    }
    
    
}



    

//
struct OneMessageSentView: View {
    
    // Stores the message datastructure
    var message: Message
    var showDate: Bool = true
    
    var body: some View {
        
        // date \ message HStack
        VStack () {
            
            // message   time
            HStack {
                Text(message.content!)
                    .padding(.horizontal, 10.0)
                    .background(.blue)
                    .frame(width: UIScreen.main.bounds.size.width / 1.5, alignment: .bottomTrailing)
                if showDate {
                    Spacer()
                    Text(timestampFormatter.string(from: message.date!))
                        .padding(.horizontal, 10.0)
                        .frame(width: UIScreen.main.bounds.size.width / 3.5, alignment: .bottomTrailing)
                }
            }
            .padding([.top, .leading, .bottom], 10)
            
        }.padding(0)
        
        
    }
}



struct OneMessageRecieved: View {
    
    // Stores the message for
    var message: Message
    var showDate: Bool = true
    
    var body: some View {
        
        // date \ message HStack
        VStack () {
            
            // message   time
            HStack {
                Text(message.content!)
                    .padding(.horizontal, 10.0)
                    .background(.gray)
                    .frame(width: UIScreen.main.bounds.size.width / 1.5, alignment: .leading)
//                    .cornerRadius(10)
                if showDate {
                    Spacer()
                    Text(timestampFormatter.string(from: message.date!))
                        .padding(.horizontal, 10.0)
                        .frame(width: UIScreen.main.bounds.size.width / 3.5, alignment: .bottomTrailing)
                }
            }
            .padding([.top, .leading, .bottom], 10)
            
        }.padding(0)
    }
}


struct OneMessageView: View {
    
    var body: some View {
        Text("tmp")
    }
}


// View for displaying the messages
struct MessageComponentView: View {
    
    // Stores the message datastructure
    var message: Message
    var mac: String
    var showDate: Bool = true
    
    var body: some View {
        // Display the message
        if message.mac == mac {
            if message.recieved {
                OneMessageRecieved(message: message, showDate: showDate)
            }
            else {
                OneMessageSentView(message: message, showDate: showDate)
            }
        }
    }
}


private let timestampFormatter: DateFormatter = {
    let formatter = DateFormatter()
    formatter.dateStyle = .short
    formatter.timeStyle = .short
    return formatter
}()


@available(iOS 16.0, *)
struct MessageView_Previews: PreviewProvider {
    static var previews: some View {
        MessageView().environment(\.managedObjectContext, PersistenceController.preview.container.viewContext)
    }
}
