# RogueSpace

## Demo Notes
**In addition** we have created a custom loader for the level and added this to the 
asset manager.  This is not necessarily the best way to do this.  It may just be best
to read in the JSON file as an asset and then have the level model parse the JSON
(which is effectively what we are doing here).  But there is a generic loader for any
class that is a subclass of `Asset`.  That is an abstract class with the following methods

* `preload`, which loads the asset on the asset thread
* `materialize` which finalizes the asset on the main render thread
* `unload` which unloads the asset
