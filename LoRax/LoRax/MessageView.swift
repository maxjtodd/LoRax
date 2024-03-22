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

    // Get the latest messages first
    // TODO: only for the mac address used
    @FetchRequest(
        sortDescriptors: [
            NSSortDescriptor(keyPath: \Message.date, ascending: true)
        ],
        animation: .default)
    private var items: FetchedResults<Message>
    
    // State variable: what is stored in the Message... box
    @State private var messageBoxContent:String = ""

    var body: some View {

        NavigationView{
            VStack {
                
                // Display the messages
                ScrollView {

                    ForEach(Array(zip(items.indices, items)), id: \.0) { i, m in
                        if m.mac == currentMac {
                            if m.recieved {
                                OneMessageRecieved(message: m)
                            }
                            else {
                                OneMessageSentView(message: m)
                            }
                        }
                    }
                }
                
                // Bottom row (send message, coordinates, other button)
                HStack {
                    // Display the chat box
                    TextField("Message...", text: $messageBoxContent)
                        .padding(.horizontal, 30)
                        .onSubmit {
                            addMessage(recieved: false)
//                            $messageBoxContent = ""
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
            // Navigation bar
            .navigationTitle("\(currentMac)")
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
                ToolbarItemGroup(placement: .topBarTrailing) {
                    Spacer()
                    Button("", systemImage: "square.and.pencil") {
                        print("Pressed")
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
    private func deleteMessage(index: Int) {
        withAnimation {
//            offsets.map { items[$0] }.forEach(viewContext.delete)
            viewContext.delete(items[index])

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



//
struct OneMessageSentView: View {
    
    // Stores the message datastructure
    var message: Message
    
    var body: some View {
        
        // date \ message HStack
        VStack () {
            
            // message   time
            HStack {
                Text(message.content!)
                    .padding(.horizontal, 10.0)
                    .background(.blue)
                    .frame(width: UIScreen.main.bounds.size.width / 1.5, alignment: .bottomTrailing)
                Spacer()
                Text(timestampFormatter.string(from: message.date!))
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
                Text(message.content!)
                    .padding(.horizontal, 10.0)
                    .background(.gray)
                    .frame(width: UIScreen.main.bounds.size.width / 1.5, alignment: .leading)
//                    .cornerRadius(10)
                Spacer()
                Text(timestampFormatter.string(from: message.date!))
                    .padding(.horizontal, 10.0)
                    .frame(width: UIScreen.main.bounds.size.width / 3.5, alignment: .bottomTrailing)
            }
            .padding([.top, .leading, .bottom], 10)
            
        }.padding(0)
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
