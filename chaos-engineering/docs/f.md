
13 Chaos engineering (for) people
This chapter covers
Mindset shifts required for effective chaos engineering
Getting buy-in from the team and management for doing chaos engineering
Applying chaos engineering to teams to make them more reliable
Let’s focus on the other type of resource that’s necessary for any project to succeed: people. In many ways, human beings and the networks we form are more complex, dynamic, and harder to diagnose and debug than the software we write. Talking about chaos engineering without including all that human complexity would therefore be incomplete.

In this chapter, I would like to bring to your attention three facets of chaos engineering meeting human brains.

First, we’ll discuss what kind of mindset is required to be an effective chaos engineer, and why sometimes that shift is hard to make.
Second is the hurdle to get a buy-in from the people around you. We will see how to communicate clearly the benefits of this approach.
Finally, we’ll talk about human teams as distributed systems and how we can apply the same chaos engineering approach we did with machines to make teams more resilient.
If that sounds like your idea of fun, we can be friends. First stop: the chaos engineering mindset.

13.1 Chaos engineering mindset
Find a comfortable position, lean back, and relax. Take control of your breath and try taking in deep, slow breaths through your nose, and release the air with your mouth. Now, close your eyes and try to not think about anything. I bet you found that hard, thoughts just keep coming. Don’t worry, I’m not going to pitch to you my latest yoga and mindfulness classes (they’re all sold out for the year)!. I just want to bring to your attention that a lot of what you consider “you” is happening without your explicit knowledge. From the chemicals produced inside of your body to help process the food you ate and make you feel sleepy at the right time of the night to split-second, subconscious decisions on other people’s friendliness and attractiveness based on visual cues, we’re all a mix of rational decisions and rationalizing the automatic ones.

To put it differently, parts of what makes up your identity are coming from the general-purpose, conscious parts of the brain, while others are coming from the subconscious. The conscious brain is much like implementing things in software - easy to adapt to any type of problem, but more costly and slow. That’s opposed to the quicker, cheaper, and more-difficult-to-change logic implemented in the hardware.

One of the interesting aspects of this duality is our perception of risk and rewards. We are capable of making the conscious effort to think about and estimate risks, but a lot of this estimation is done automatically without even reaching the level of consciousness. And the problem is that some of these automatic responses might still be optimized for surviving in the harsh environments the early human was exposed to, and not doing computer science.

Chaos engineering mindset is all about estimating risks and rewards with partial information, instead of relying on the automatic responses and gut feelings. It requires doing things that feel counterintuitive at first, like introducing failure into computer systems, after a careful consideration of the risk-reward ratio. It necessitates a scientific, evidence-based approach, coupled with a keen eye for potential problems. In the rest of this chapter I will try to illustrate why.

CALCULATING RISKS - THE TROLLEY PROBLEM
If you think that you’re good at risk mathematics, think again. You might be familiar with the Trolley problem (https://en.wikipedia.org/wiki/Trolley_problem). In the experiment, participants are asked to make a choice which will affect other people - either keep them alive, or not. There is a trolley barrelling down the tracks. Ahead, there are five people attached to the tracks. If the trolley hits them, they die. You can’t stop the trolley and there is not enough time to detach even a single person. However, you notice a lever, Pulling the lever will redirect the trolley to another set of tracks, which only has one person attached to it. What do you do?

You might think that most people would calculate that one person dying is better than five people dying, and pull the lever. But the reality is that most people wouldn’t do it. There is something about it that makes the basic arithmetics go out of the window.

Let’s take a look at the mindset of an effective chaos engineering practitioner, starting with failure.

13.1.1   Failure is not a maybe: it will happen
Let’s assume that we’re using good-quality servers. One way of expressing the quality of being “good quality” in a scientific manner is the mean time between failure (MTBF). For example, if the servers had a very long MTBF of 10 years, that means that on average, each of them would fail every 10 years. Or put differently, the probability of the machine failing today is 1/(10 years * 365.25 days in a year) ~= 0.0003, or 0.03%. If we’re talking about the laptop I’m writing these words on, I am only 0.03% worried it will die on me today.

The problem is that small samples like this give us a false impression of how reliable things really are. Imagine now a datacenter with 10,000 servers in it. How many servers can they expect to fail on any given day? It’s 0.0003 * 10,000 ~= 3. Even with a third of that, at 3333 servers, the number would be 0.0003 * 3,333 ~= 1. The scale of modern systems we’re building makes small error rates like this more pronounced, but as you can see you don’t need to be Google or Facebook to experience them.

Once you get a hang of it, multiplying percentages is fun. Here’s another example. Let’s say that you have a mythical, all-star team shipping bug-free code 98% of the time. That means, on average, that with a weekly release cycle, they will ship bugs more than once a year. And if your company has 25 teams like that, all 98% bug free, you’re going to have a problem every other week, again, on average.

In the practice of chaos engineering, it’s important to look at things from this perspective - a calculated risk - and to plan accordingly. Now, with these well-defined values and elementary school-level multiplication, we can estimate a lot of things and make informed decisions. But what happens if the data is not readily available, and it’s harder to put a number to it?

13.1.2   Failing early vs failing late

One common mindset blocker when starting with chaos engineering is that we might cause an outage that we would otherwise most likely get away with. We discussed how to minimize this risk in the chapter on testing in production, so now I’d like to focus on the mental part of the equation. The reasoning tends to be like this: “It’s currently working, its lifespan is X years, so chances are that even if it has bugs that would be uncovered by chaos engineering, we might not run into them within this lifespan.”

There are many reasons why a person might think this. The company culture could be punitive for mistakes. They might have had software running in production for years, and bugs were found only when it was being decommissioned. Or simply because they have low confidence in their (or someone else’s) code. And there may be plenty of other reasons.

A universal reason, though, is that we have a hard time comparing two probabilities we don’t know how to estimate. Because an outage is an unpleasant experience, we’re wired to overestimate how likely it is to happen. It’s the same mechanism that makes people scared of dying of shark attack. In 2019 two people died of shark attacks in the entire world (https://www.floridamuseum.ufl.edu/shark-attacks/trends/location/world/.) Given the estimated population of 7.5B people in June 2019 (https://www.census.gov/popclock/world) the likelihood of any given person dying from a shark attack that year was 1 in 3,250,000,000. But because people watched the movie Jaws, if interviewed in the street, they will estimate that likelihood very high.

Unfortunately, this just seems to be how we are. So instead of trying to convince people to swim more in shark waters, let’s change the conversation. Let’s talk about the cost of failing early versus the cost of failing late. In the best-case scenario (from the perspective of possible outages, not learning), chaos engineering doesn’t find any issues, and all is good. In the worst-case scenario the software is faulty. If we experiment on it now, we might cause the system to fail and affect users within our blast radius. We call this failing early. If we don’t experiment on it, it’s still likely to fail, but possibly much later on (failing late).

Failing early has a number of advantages:

There are engineers actively looking for bugs, with tools at the ready to diagnose the issue and help fix it as soon as possible. Failing late might happen at a much less convenient time.
The same applies to the development team. The further in the future from the code being written, the bigger the context switch the person fixing the bug will have to execute.
As a product (or company) matures, usually the users expect to see increased stability and decreased number of issues over time.
Over time, the number of dependent systems tends to increase.
But because you’re reading this book, chances are you’re already aware of the advantages of doing chaos engineering and failing early. The next hurdle is to get the people around you to see the light too. Let’s take a look at how to achieve that in the most effective manner.


13.2 Getting the buy-in
Jn erdor kr uor pthv osmr tmlk vtsx vr soahc neniigrgeen yvtx, gqe xnkb ormg rx denurnatds krg eitesfnb rj bginsr. Cpn nj oedrr ltk rqmo er ardnuedtns seoht teifsben, eyq vynv rv qk fkzu re oeiumtamccn kmdr llarcey. Xbtkx tks capytlily vwr oupsgr vl opepel ugx’tk inggo kr xh tinpcghi xr: thkh t/reempase mrsebem uzn dbxt ntnmaemgae. Prk’c srtat qu kglnoio rc kpw er xrzf kr xqr rttale.

13.2.1   Management
Put yourself in your manager’s shoes. The more projects you’re responsible for, the more likely you are to be risk-averse. After all, what you want is to minimize the number of fires to extinguish, while achieving your long-term goals. And chaos engineering can help with that.

So in order to play some music to your manager's ears, perhaps don’t start with breaking things on purpose in production. Here are some elements they are much more likely to react well to:

Good return on investment (ROI): chaos engineering can be a relatively cheap investment (even a single engineer can experiment on a complex system in a single-digit number of days if the system is well documented) with a big potential pay off. The result is a win-win situation:
If the experiments don’t uncover anything, the output is twofold. First increased confidence in the system. Second, a set of automated tests which can be re-run to detect any regressions later.
If the experiments uncover a problem, it can be fixed.
Controlled blast radius: it’s good to remind them again, that this is not going to be randomly breaking things, but a well-controlled experiment, with a defined blast radius. Obviously, things can still go sideways, but the idea is not to set the world on fire and see what happens. Rather, it’s to take a calculated risk for a large potential pay off.
Failing early:
Cost of resolving an issue found earlier is generally lower than if the same issue was found later.
Faster response time to an issue found on purpose, rather than at an inconvenient time.

Better-quality software:
Your engineers knowing the software will get experimented are more likely to think about the failure scenarios early in the process and write more resilient software.
Team building: the increased awareness of the importance of interaction and knowledge sharing has the potential to make teams stronger. More on this later in this chapter.
Increased hiring potential:
A real-life proof of building solid software. All companies talk about the quality of their product. Only a subset puts their money where their mouth is when it comes to funding engineering efforts in testing.
Solid software means fewer calls outside of working hours, which means happier engineers.
Shininess factor. Using the latest techniques helps attract engineers who want to learn them and have them on their CVs.

If delivered correctly, the tailored message should be an easy sell. It has the potential to make your manager’s life easier, make the team stronger, the software better quality, and hiring easier. Why would you not do chaos engineering?!

How about your team members?

13.2.2   Team members
When speaking to your team members, many of the arguments we just covered apply in an equal measure. Failing early is less painful than failing later, thinking about corner cases and designing all software with failure in mind is often interesting and rewarding. Oh and office games (we’ll get to them in just a minute) are fun.

But often what really resonates with the team is simply the potential of getting called less. If you’re on an on-call rotation, everything that minimizes the number of times you’re called in the midst of a night is helpful. So framing the conversation around this angle can really help with getting them onboard. Here are some ideas of how to approach that conversation:

Failing early and during work hours: If there is an issue, it’s better to trigger it before you’re about to go pick up your kids from school or go to sleep in the comfort of your own home.
Destigmatising failure: Even for a rock-star team, failure is inevitable. Thinking about it and actively seeking problems can remove or minimize the social pressure of not failing. Learning from failure always trumps avoiding and hiding failure. Conversely, for a poorly performing team, the failure is likely a common occurrence. Chaos engineering can be used in pre-production stages as an extra layer of testing, allowing the unexpected failures to be more rare.
Chaos engineering is a new skill, and one that’s not that hard to pick up: Personal improvement will be a reward in itself for some. And it’s a new item on a CV.
With that, you should be well-equipped to evangelize chaos engineering within your teams and to your bosses. You can now go and spread the good word! But before you go, let me give you one more tool. Let’s talk about game days.

13.2.3   Game days
You might have heard of teams running game days. Game days are a good tool for getting the buy-in from the team. They are a little bit like those events at your local car dealership. Big, colorful balloons, free cookies, plenty of test drives and miniature cars for your kid, and boom--all of a sudden you need a new car. It’s like a gateway drug, really.

Game days can take any form. The form is not important. The goal is to get the entire team to interact, brainstorm some ideas of where the weaknesses of the system might lie, and have some fun with chaos engineering. It’s both the balloons and the test drives that make you want to use a new chaos engineering tool.

You can set up recurring game days. You can start your team off with a single event to introduce them to the idea. You can buy some fancy cards for writing down chaos experiment ideas, or you can use sticky notes. Whatever you think will get your team to appreciate the benefits, without feeling like it’s forced upon them, will do. Make them feel they’re not wasting their time. Don’t waste their time.

That’s all I have for the buy-in -- time to dive a level deeper. Let’s see what happens if you apply chaos engineering to a team itself.

13.3 Teams as distributed systems
What’s a distributed system? Wikipedia defines it as “a system whose components are located on different networked computers, which communicate and coordinate their actions by passing messages to one another” (https://en.wikipedia.org/wiki/Distributed_computing). If you think about it, a team of people behaves like a distributed system, but instead of computers we have individual humans doing things and passing messages to one another.

Let’s imagine a team responsible for running customer-facing ticket-purchasing software for an airline. The team will need varied skills to succeed and because it’s a part of a larger organization, some of the technical decisions will be made for them. Let’s take a look at a concrete example of the core competences required for this team:

Microsoft SQL database cluster management. That’s where all the purchase data lands and that’s why it is crucial to the operation of the ticket sales. This also includes installing and configuring Windows OS on VMs.
Full-stack Python development. For the backend receiving the queries about available tickets as well as the purchase orders, this also includes packaging the software and deploying it on Linux VMs. Basic Linux administration skills are therefore required.
Front-end, JavaScript-based development.
Design. Providing artwork to be integrated into the software by the front-end developers.
Integration with third-party software. Often, the airline can sell a flight operated by another airline, so the team needs to maintain integration with other airlines’ systems. What it entails varies from case to case.


Now, the team is made of individuals, all of whom have accumulated a mix of various skills over time as a function of their personal choices. Let’s say that some of our Windows DB admins are also responsible for integrating with third-parties (the Windows-based systems, for example). Similarly, some of the full stack developers also handle the integrations with Linux-based third-parties. Finally, some of the front-end developers can also do some design work. Take a look at figure 13.1, which shows a Venn diagram of these skills overlaps.

Figure 13.1 Venn diagram of skills overlap in our example team

The team is also lean. In fact, there are only six people on the team. Alice and Bob are both Windows and Microsoft SQL experts. Alice also supports some integrations. Caroline and David are both full stack developers, and both work on integrations. Esther is a front-end developer who can also do some design work. Franklin is the designer. Figure 13.2 places these individuals onto the Venn diagram of the skills overlaps.

Figure 13.2 Individuals on the Venn diagram of skills’ overlap

Can you see where I’m going with this? Just like with any other distributed system, we can identify the weak links by looking at the architecture diagram. Do you see any weak links? For example, if Esther has a large backlog, there is no one else on the team who can pick it up, because no one else has the skills. She’s a single point of failure. By contrast, if one of Caroline or David is distracted with something else, the other one can cover: they have redundancy between them. People need holidays, they get sick, and they change teams and companies, so in order for the team to be successful long term, identifying and fixing single points of failure is very important. It’s pretty convenient we had a Venn diagram ready!

One problem with real life is that it’s messy. Another is that teams rarely come nicely packaged with a Venn diagram attached to the box. Hundreds of different skills (hard and soft), constantly shifting technological landscape, evolving requirements, personnel turnaround and a sheer scale of some organizations are all factors in how hard it can be to ensure no single points of failure. If only there was a methodology to uncover systemic problems in a distributed system... Oh wait!

In order to discover systemic problems within a team, let’s do some chaos experiments! The following experiments are heavily inspired by Dave Rensin who described them in his talk, “Chaos Engineering for People Systems” (https://youtu.be/sn6wokyCZSA). I strongly recommend watching that talk. They are also best sold to the team as “games” rather than experiments. Not everyone wants to be a guinea pig, but a game sounds like a lot of fun and can be a team-building exercise if done right. You could even have prizes!

Let’s start with identifying single points of failure within a team.

13.3.1   Finding knowledge single points of failure: “Staycation”
To see what happens to a team in the absence of a person, the chaos engineering way of verifying it is to simulate the event and observe how they cope. The most lightweight variant is to just nominate a person and ask them to not answer any queries related to their responsibilities and work on something different than they had scheduled for the day. Hence, the name staycation. Of course, it’s a game, and should an actual emergency arise, it’s called off and all hands are on deck.

If the team continues working fine at full (remaining) capacity, that’s great. It means the team is doing a really good job of spreading knowledge. But chances are that there will be occasions where other team members need to wait for the person on staycation to come back, because some knowledge wasn’t replicated sufficiently. It could be some work in progress that wasn’t documented well enough, some area of expertise that suddenly became relevant, some tribal knowledge the newer people on the team don’t have yet, or any number of other reasons. If that’s the case, congratulations: you’ve just discovered how to make your team stronger as a system!

People are different, and some will enjoy games like this much more than others. You’ll need to find something that works for the individuals on your team. There is not a single best way of doing this; whatever works is fair game. Here are some other knobs to tweak in order to create an experience better tailored for your team:

Unlike a regular vacation, where the other team members can predict problems and execute some knowledge transfer to avoid them, it might be interesting to run this game by surprise. It will simulate someone falling sick, rather than taking a holiday.

You can tell the other team members about the experiment... or not. Telling them will have an advantage that they can proactively think about things they won’t be able to resolve without the person on staycation. Telling them only after the fact is closer to a real-life situation, but might be seen as distraction. You know your team, suggest what you think will work best.
Choose your timing wisely. If the team is working hard to meet a deadline, they might not enjoy playing games that eat up their time. Or, if they are very competitive, they might like that, and with the higher number of things going on there might be more potential for knowledge sharing issues to arise.
Whichever way works for your team, this can be a really inexpensive investment with a large potential payout. Make sure you take the time to discuss the findings with the team, lest they might find the game unhelpful. Everyone involved is an adult and should recognize when there is a real emergency. But even if the game went too far, failing early is most likely better than failing late, just like with the chaos experiments we run in computer systems.

Let’s take a look at another variant, this time focusing not on absence, but on false information.

13.3.2   Misinformation and trust within the team: “Liar, liar”
In a team, information flows from one team member to another. There needs to be a certain amount of trust between the members for effective cooperation and communication, but also a certain amount of distrust, so that we double-check and verify things, instead of just taking them at face value. After all, to err is human.

We’re also complex human beings, and we can trust the same person more on one topic than a different one. That’s very helpful. You reading this book shows some trust in my chaos engineering expertise, but that doesn’t mean you should trust my carrot cake (the last one didn’t look like a cake, let alone a carrot!) And that’s perfectly fine, these checks should be in place so that wrong information can be eventually weeded out. We want that property of the team, and we want it strong.

“Liar, liar” is a game designed to test how well your team is dealing with false information circulating. The basic rules are simple: nominate a person who’s going to spend the day telling very plausible lies when asked about work-related stuff. Some safety measures: write down the lies, and if they weren’t discovered by others, straighten them out at the end of the day and in general be reasonable with them. Don’t create a massive outage by telling another person to click “delete” on the whole system.

What this has a potential to uncover are situations where other team members skip the mental effort of validating their inputs and just take what they heard at face value. Everyone makes a mistake, and it’s everyone’s job to reality-check what you heard before you implement it. Here are some ideas of how to customize this game:

Choose the liar wisely. The more the team relies on their expertise, the bigger the blast radius, but also the bigger the learning potential.
The liar’s acting skills are actually pretty useful here. If they can keep it up for the whole day, without spilling the beans, it should have a pretty strong wow effect with other team members.

You might want to have another person on the team know about the liar, to observe and potentially step in if they think the situation might have some consequences they didn’t think of. At a minimum, the team leader should always know about this!
Take your time to discuss the findings within the team. If people see the value in doing this, it can be good fun. Speaking of fun, do you recall what happened when we injected latency in the communications with the database in chapter 4? Let’s see what happens when you inject latency into a team.

13.3.3   Bottlenecks in the team: “life in the slow lane”
The next game, “life in the slow lane,” is about finding who’s a bottleneck within the team, in different contexts. In a team, people share their respective expertise to propel the team forward. But everyone has a maximum throughput of what they can process. Bottlenecks form, where some team members need to wait for others before they can continue with their work. In the complex network of social interactions, it’s often difficult to predict and observe these bottlenecks, until they become obvious.

The idea behind this game is to add latency to a designated team member by asking them to take at least X minutes to respond to queries from other team members. By artificially increasing the response time, you will be able to discover bottlenecks more easily: they will be more pronounced, and people might complain about them directly! Here are some tips to ponder:

If possible, working from home might be useful when implementing the extra latency. It limits the amount of social interaction and might make it a bit less weird.
Going silent when others are asking for help is suspicious, might make you uncomfortable, and can even be seen as rude. Responding to the queries with something along the lines of, “I’ll get back to you on this, sorry I’m really busy with something else right now,” might help greatly.
Sometimes resolving found bottlenecks might be the tricky bit. There might be policies in place, cultural norms or other constraints to take into account, but even just knowing about the potential bottlenecks can help planning ahead.
Sometimes, the manager of the team will be a bottleneck in some things. It might require a little bit more of self-reflection and maturity to react to that, but it can provide invaluable insights.

So this one is pretty easy, and you don’t need to remember the syntax of tc to implement it! And since we’re on a roll, let’s cover one more. Let’s see how to use chaos engineering to test out your remediation procedures.

13.3.4   Testing your processes: “inside job”
Your team, unless it was started earlier today, has a set of rules to deal with problems. These rules might be well-structured and written down; might be tribal knowledge in the collective mind of the team; or like it’s the case for most teams, somewhere in between the two. Whatever they are, these “procedures” of dealing with different types of incidents should be reliable. After all, that’s what you rely on in the stressful times. Given that you’re reading a book on chaos engineering, how do you think we could test them out?

With a gamified chaos experiment, of course! I’m about to encourage you to execute an occasional act of controlled sabotage by secretly breaking some sub-system you reasonably expect the team to be able to fix using the existing procedures, and then sit and watch then fix it.

Now, this is a big gun, so here’s a few caveats:

Be reasonable about what you break. Don’t break anything that would get you in trouble.
Pick the inside group wisely. You might want to let the stronger people on the team in on the secret, and let them “help out” by letting the other team members follow the procedures to fix the issue.
It might also be a good idea to send some people to training or a side project, to make sure that the issue can be solved even with some people out.
Double-check that the existing procedures are up to date before you break the system.
Take notes while you’re observing the team react to the situation. See what takes up their time, what part of the procedure is prone to mistakes, who might be a single point of failure during the incident.
It doesn’t have to be a serious outage. It might be a moderate severity issue, which needs to be remediated before it becomes very serious.

If done right, this can be a very useful piece of information. It increases the confidence in the team’s ability to fix an issue of a certain type. And again, it’s much nicer to be dealing with an issue just after lunch, rather than at 2am.

Would you do an inside job in production? The answer will depend on many factors we covered earlier in the chapter on testing in production and on the risk/reward calculation. In the worst-case scenario, you create an issue that the team fails to fix in time, the game needs to be called off, the issue fixed. You learn that your procedures are inadequate and can take action on improving them. In many situations, this might be perfectly good odds.

There are infinite other games you can come up with by applying the chaos engineering principles to the human teams and interaction within them. My goal here was to introduce you to some of them to illustrate that human systems have a lot of the same characteristics as computer systems. I hope that I pricked your interest. Now, go forth and experiment with and on your teams!

13.4 Summary
Chaos engineering requires a mindset shift from risk averse to risk calculating
Getting the buy-in from the team and management can be made easier through good communication and tailoring the message
Teams are distributed systems, and can also be made more reliable through the practice of experiments and office games


