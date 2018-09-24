## This file Contains some general instruction how to use Git, can be ignored if already known ##

##Installing Guide##
- Install miniconda, quick install : http://conda.pydata.org/docs/install/quick.html
- Open Anaconda cmd 
- To verify install type: conda list
- Update conda: conda update conda
- Install git: conda install -c anaconda git=2.9.3

##Set Up Your Account##
- git config --global user.name "YOUR_USERNAME"
- git config --global user.email "your_email_address@example.com"


##Generate SSH Key Pair##
- Type "ssh-keygen", follow the prompts, best to leave everything as default
- Copy the SSH key from the generated file
- Open GitLab online
- Go to "Profile Settings", "SSH Keys"
- Paste the key and save

## Inital Mapping to Workstation ##
- git clone git@gitlab.com:TeamSuperAwesome2016/MicroSolarCharger.git

## To Pull Updated files in future can just enter teh following ##
- git pull

## Creating/Adding Files in the Repository ##
- touch <file> 
- git add <file>
- git commit -m "add <file>" 

# - Should be a general comment as to what is being done
- git push -u origin master

## Modifying Files in the Reporitory ##
- git checkout -b "<name>Branch"
# - Create a new branch so that files are not pushed to master directly. This is done so that push/merge changes do not happen accidentally without review
- git add <file/Folder>
- git commit -m "Comment"
- git push --set-upstream origin <name>Branch
# - Push branch to remote. Subsequent push from the same branch won't need the "--set-upstream" value.

## Add to an Existing Folder or Git Repository ##
- cd existing_folder
- git init
- git remote add origin git@gitlab.com:TeamSuperAwesome2016/MicroSolarCharger.git
- git add .
- git commit
- git push -u origin master
