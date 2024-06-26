//
//  Persistence.swift
//  TestCoreData
//
//  Created by Max Todd on 3/14/24.
//

import CoreData

struct PersistenceController {
    static let shared = PersistenceController()

    // For previews - create objects to store in DB
    static var preview: PersistenceController = {
        let result = PersistenceController(inMemory: true)
        let viewContext = result.container.viewContext
        for i in 0..<10 {
            
            // create messages
            let newMessage = Message(context: viewContext)
            newMessage.mac = "A1:B2:C3:D4:E5:F1"
            newMessage.content = "hello world"
            newMessage.date = Date()
            if (i % 2 == 0) {
                newMessage.recieved = true
            }
            else {
                newMessage.recieved = false
            }
            newMessage.id = UUID()
            

        }
        do {
            try viewContext.save()
        } catch {
            // Replace this implementation with code to handle the error appropriately.
            // fatalError() causes the application to generate a crash log and terminate. You should not use this function in a shipping application, although it may be useful during development.
            let nsError = error as NSError
            fatalError("Unresolved error \(nsError), \(nsError.userInfo)")
        }
        
        for i in 0..<10 {
            // create contact
            let newContact = Contact(context: viewContext)
            newContact.mac = "A1:B2:C3:D4:E5:F\(i)"
            newContact.fName = "User\(i)"
            newContact.lName = "US\(i)"
            newContact.id = UUID()
        }
        do {
            try viewContext.save()
        } catch {
            // Replace this implementation with code to handle the error appropriately.
            // fatalError() causes the application to generate a crash log and terminate. You should not use this function in a shipping application, although it may be useful during development.
            let nsError = error as NSError
            fatalError("Unresolved error \(nsError), \(nsError.userInfo)")
        }
        
        for i in 0..<3 {
            // create non contact
            let newNonContact = NonContact(context: viewContext)
            newNonContact.id = UUID()
            newNonContact.mac = "F1:B2:C3:D4:E5:F\(i)"
        }
        do {
            try viewContext.save()
        } catch {
            // Replace this implementation with code to handle the error appropriately.
            // fatalError() causes the application to generate a crash log and terminate. You should not use this function in a shipping application, although it may be useful during development.
            let nsError = error as NSError
            fatalError("Unresolved error \(nsError), \(nsError.userInfo)")
        }
        
        return result
    }()

    let container: NSPersistentContainer

    init(inMemory: Bool = false) {
        container = NSPersistentContainer(name: "LoRax")
        if inMemory {
            container.persistentStoreDescriptions.first!.url = URL(fileURLWithPath: "/dev/null")
        }
        container.loadPersistentStores(completionHandler: { (storeDescription, error) in
            if let error = error as NSError? {
                // Replace this implementation with code to handle the error appropriately.
                // fatalError() causes the application to generate a crash log and terminate. You should not use this function in a shipping application, although it may be useful during development.

                /*
                 Typical reasons for an error here include:
                 * The parent directory does not exist, cannot be created, or disallows writing.
                 * The persistent store is not accessible, due to permissions or data protection when the device is locked.
                 * The device is out of space.
                 * The store could not be migrated to the current model version.
                 Check the error message to determine what the actual problem was.
                 */
                fatalError("Unresolved error \(error), \(error.userInfo)")
            }
        })
        container.viewContext.automaticallyMergesChangesFromParent = true
    }
}
