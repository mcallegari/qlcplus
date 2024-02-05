If you are a developer and you want to help to improve QLC+, then thanks for that !

This page is meant to provide some guidelines about how to approach the QLC+ project, and to provide what is required to contribute to the code without messing things up or wasting the maintainers limited time.

The fundamental things you need to keep in mind are: 
- QLC+ is a complex C++/Qt software
- QLC+ is cross platform
- QLC+ has a quite large user base, with a wide variety of usage cases

Given the above, the guidelines you are invited to follow are:
- every change must be submitted via GitHub, so if you're not confident with GIT, please read some documentation about how to submit a clean pull request. There's plenty of literature online
- the fact QLC+ is open source, doesn't necessarily mean anyone can change the code
- if you are an amateur developer or even if you are an expert developer, but on languages other than C++, then before proposing a change you should spend some time studying the existing code, especially the part you intend to change. There are hundreds of thousands of lines of code where you can learn from
- if you found something that you don't like, it doesn't necessarily mean it has to be changed, cause other users might not be happy about changing it
- if you changed something and it works for you, it doesn't mean it will work on every platform supported by QLC+. This is especially true for platform-related topics, like audio/video code, timers, plugins, UI customizations. Therefore you are invited to test your changes on more than one platform, where Windows is the most used
- if you change something in a very specific area, it doesn't mean it will not have reflections (and maybe cause regressions) on other areas. It's hard to have the whole picture of how QLC+ works, but if you have doubts, just ask in the [forum development section](http://www.qlcplus.org/forum/viewforum.php?f=12)
- if you want to add new options, then ask yourself if it is really needed, cause newbie users might be confused about so many options and might not be encouraged to adopt QLC+
- new options should also be documented in HTML form. If you don't do it, then remember someone else will have to, thus spending their time instead of yours
- QLC+ uses Qt test unit classes, so before submitting a change, you should run `make check` and double check your changes didn't introduce regressions. If one or more tests don't pass, please fix your code before creating a pull request
- please follow the coding style adopted all over the QLC+ code. Especially parenthesis and spaces
- changes to the engine classes or the VCWidget class **must be carefully discussed first** !
- please try to take responsibilities for the changes you make. Do not just throw lines of code in without careful tests and hope someone else will test it better than you or take responsibility for your code

In general there are cases and cases. If you changed just a few lines of code, most likely the review will be quick and the possibility to be merged are high.
However, if you changed dozens or hundreds of lines, all the above are applied. The more you explain what you did and why you did it, the less it will take for a review and an eventual discussion.
