We call "the active visitor" as the last visitor logged in.

Possible commands:

1. login [visitor name]
   
   Only authorized visitors can log in. The shopkeeper with name "Shopkeeper" is authorized since the system was first used.

2. logout
   
   The active visitor should log out after this command.

3. querybook --ISBN=[book ISBN] --book-name=[book name] --author-name=[author name] --keywords=[keywords]
   
   Client access required.
   
   Important: Use commas ',' to separate keywords. No spaces allowed.
   
   Echoes the full information about books that meets these requirements.

4. purchasebook [book ISBN] [purchase number]
   
   Client access required.
   
   The active visitor purchases a given number of the given book.

5. restock [book ISBN] [purchase number] [price]
   
   Staff access required.
   
   The active staff restocks the given book at a given number and a given price.

6. registerbook [book ISBN] [book name] [author name] [keyword]+
   
   Staff access required.
   
   The active staff registers a book with full information.

7. modifybook [book ISBN] --book-name=[book name] --author-name=[author name] --keywords=[keywords]
   
   Staff access required.
   
   Important: Use commas ',' to separate keywords. No spaces allowed.
   
   The active staff modifys the book in the library with given information.

8. visitorregister [visitor name] [access level]
   
   Staff access required.
   
   "access level" should be among "guest", "client" and "staff".
   
   Give a visitor an access.

9. Log [--all] 
   
   to be continued
   
   














